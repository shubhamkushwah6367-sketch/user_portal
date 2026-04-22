#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

using namespace std;

struct Student {
    int    id;
    string name;
    string username;
    string password;
    string branch;
    int    year;
    float  cgpa;
    string email;
};

static const string DB_FILE = "students.json";
static const string INDEX_FILE = "student_portal.html";
static const int SERVER_PORT = 8080;

vector<Student> students;
int nextId = 1001;

string trim(const string& value) {
    size_t start = 0;
    while (start < value.size() && isspace((unsigned char)value[start])) start++;
    size_t end = value.size();
    while (end > start && isspace((unsigned char)value[end - 1])) end--;
    return value.substr(start, end - start);
}

string toLower(string value) {
    transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) { return tolower(ch); });
    return value;
}

bool fileExists(const string& path) {
    ifstream input(path, ios::binary);
    return input.good();
}

bool readFile(const string& path, string& output) {
    ifstream input(path, ios::binary);
    if (!input.is_open()) return false;
    ostringstream buffer;
    buffer << input.rdbuf();
    output = buffer.str();
    return true;
}

bool writeFile(const string& path, const string& contents) {
    ofstream output(path, ios::binary);
    if (!output.is_open()) return false;
    output << contents;
    return true;
}

string escapeJson(const string& value) {
    string result;
    for (char ch : value) {
        switch (ch) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result.push_back(ch);
        }
    }
    return result;
}

string extractJsonString(const string& json, const string& key) {
    auto pos = json.find('"' + key + '"');
    if (pos == string::npos) return "";
    auto colon = json.find(':', pos);
    if (colon == string::npos) return "";
    size_t index = colon + 1;
    while (index < json.size() && isspace((unsigned char)json[index])) index++;
    if (index >= json.size() || json[index] != '"') return "";
    index++;
    string value;
    while (index < json.size()) {
        if (json[index] == '"') break;
        if (json[index] == '\\' && index + 1 < json.size()) {
            index++;
            if (json[index] == '"') value.push_back('"');
            else if (json[index] == '\\') value.push_back('\\');
            else if (json[index] == 'n') value.push_back('\n');
            else if (json[index] == 'r') value.push_back('\r');
            else if (json[index] == 't') value.push_back('\t');
            else value.push_back(json[index]);
        } else {
            value.push_back(json[index]);
        }
        index++;
    }
    return value;
}

template <typename T>
T extractJsonNumber(const string& json, const string& key, T fallback) {
    auto pos = json.find('"' + key + '"');
    if (pos == string::npos) return fallback;
    auto colon = json.find(':', pos);
    if (colon == string::npos) return fallback;
    size_t index = colon + 1;
    while (index < json.size() && isspace((unsigned char)json[index])) index++;
    if (index >= json.size()) return fallback;
    size_t start = index;
    while (index < json.size() && (isdigit((unsigned char)json[index]) || json[index] == '.' || json[index] == '-' || json[index] == '+')) {
        index++;
    }
    if (start >= index) return fallback;
    try {
        string token = json.substr(start, index - start);
        if (is_same<T, int>::value) return static_cast<T>(stoi(token));
        if (is_same<T, float>::value) return static_cast<T>(stof(token));
        if (is_same<T, double>::value) return static_cast<T>(stod(token));
    } catch (...) {
        return fallback;
    }
    return fallback;
}

bool loadDatabase() {
    if (!fileExists(DB_FILE)) return true;
    string raw;
    if (!readFile(DB_FILE, raw)) return false;
    students.clear();
    size_t position = 0;
    while (true) {
        position = raw.find('{', position);
        if (position == string::npos) break;
        size_t end = raw.find('}', position);
        if (end == string::npos) break;
        string block = raw.substr(position, end - position + 1);
        Student student;
        student.id = extractJsonNumber<int>(block, "id", 0);
        student.name = extractJsonString(block, "name");
        student.username = extractJsonString(block, "username");
        student.password = extractJsonString(block, "password");
        student.email = extractJsonString(block, "email");
        if (student.id > 0 && !student.username.empty()) {
            students.push_back(student);
        }
        position = end + 1;
    }
    int maxId = 1000;
    for (const auto& s : students) {
        if (s.id > maxId) maxId = s.id;
    }
    nextId = maxId + 1;
    return true;
}

bool saveDatabase() {
    ostringstream output;
    output << "[\n";
    for (size_t i = 0; i < students.size(); ++i) {
        const Student& s = students[i];
        output << "  {\n";
        output << "    \"id\": " << s.id << ",\n";
        output << "    \"name\": \"" << escapeJson(s.name) << "\",\n";
        output << "    \"username\": \"" << escapeJson(s.username) << "\",\n";
        output << "    \"password\": \"" << escapeJson(s.password) << "\",\n";
        output << "    \"email\": \"" << escapeJson(s.email) << "\"\n";
        output << "  }";
        if (i + 1 < students.size()) output << ",\n";
        else output << "\n";
    }
    output << "]\n";
    return writeFile(DB_FILE, output.str());
}

string makeJsonSafeStudent(const Student& student, bool includePassword = false) {
    ostringstream output;
    output << "{\n";
    output << "  \"id\": " << student.id << ",\n";
    output << "  \"name\": \"" << escapeJson(student.name) << "\",\n";
    output << "  \"username\": \"" << escapeJson(student.username) << "\",\n";
    if (includePassword) {
        output << "  \"password\": \"" << escapeJson(student.password) << "\",\n";
    }
    output << "  \"email\": \"" << escapeJson(student.email) << "\"\n";
    output << "}";
    return output.str();
}

string makeStudentListJson() {
    ostringstream output;
    output << "[\n";
    for (size_t i = 0; i < students.size(); ++i) {
        output << makeJsonSafeStudent(students[i]);
        if (i + 1 < students.size()) output << ",\n";
        else output << "\n";
    }
    output << "]\n";
    return output.str();
}

string jsonError(const string& message) {
    ostringstream output;
    output << "{\n  \"error\": \"" << escapeJson(message) << "\"\n}";
    return output.str();
}

string jsonSuccess(const string& message) {
    ostringstream output;
    output << "{\n  \"success\": true,\n  \"message\": \"" << escapeJson(message) << "\"\n}";
    return output.str();
}

bool usernameExists(const string& username) {
    auto lower = toLower(username);
    return any_of(students.begin(), students.end(), [&](const Student& student) {
        return toLower(student.username) == lower;
    });
}

Student* findStudentByUsername(const string& username) {
    auto lower = toLower(username);
    for (auto& student : students) {
        if (toLower(student.username) == lower) {
            return &student;
        }
    }
    return nullptr;
}

string buildResponse(const string& status, const string& contentType, const string& body) {
    ostringstream response;
    response << "HTTP/1.1 " << status << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Headers: Content-Type\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response << "\r\n";
    response << body;
    return response.str();
}

void initializeSockets() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

void cleanupSockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

bool sendAll(int socket, const string& data) {
    size_t total = 0;
    while (total < data.size()) {
        int sent = ::send(socket, data.data() + total, static_cast<int>(data.size() - total), 0);
        if (sent <= 0) return false;
        total += sent;
    }
    return true;
}

bool receiveAll(int socket, string& request, string& body, map<string, string>& headers) {
    request.clear();
    body.clear();
    headers.clear();

    char buffer[8192];
    string raw;
    size_t headerEnd = string::npos;

    while (true) {
        int bytes = recv(socket, buffer, sizeof(buffer), 0);
        if (bytes <= 0) break;
        raw.append(buffer, bytes);
        headerEnd = raw.find("\r\n\r\n");
        if (headerEnd != string::npos) break;
    }

    if (headerEnd == string::npos) return false;
    string headerText = raw.substr(0, headerEnd);
    body = raw.substr(headerEnd + 4);

    istringstream stream(headerText);
    string line;
    bool firstLine = true;
    while (getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (firstLine) {
            request = line;
            firstLine = false;
            continue;
        }
        auto colon = line.find(':');
        if (colon != string::npos) {
            string name = trim(line.substr(0, colon));
            string value = trim(line.substr(colon + 1));
            headers[toLower(name)] = value;
        }
    }

    auto it = headers.find("content-length");
    if (it != headers.end()) {
        int expectedLength = stoi(it->second);
        while ((int)body.size() < expectedLength) {
            int bytes = recv(socket, buffer, sizeof(buffer), 0);
            if (bytes <= 0) break;
            body.append(buffer, bytes);
        }
    }

    return true;
}

string routeApiRegister(const string& requestBody) {
    string name = trim(extractJsonString(requestBody, "name"));
    string username = trim(extractJsonString(requestBody, "username"));
    string password = extractJsonString(requestBody, "password");
    string email = trim(extractJsonString(requestBody, "email"));

    if (name.empty() || username.empty() || password.empty() || email.empty()) {
        return jsonError("Please fill all fields with valid values.");
    }
    if (password.size() < 6) {
        return jsonError("Password must be at least 6 characters long.");
    }
    if (usernameExists(username)) {
        return jsonError("Username already exists. Choose a different username.");
    }

    Student student;
    student.id = nextId++;
    student.name = name;
    student.username = username;
    student.password = password;
    student.branch = "";
    student.year = 1;
    student.cgpa = 0.0f;
    student.email = email;

    students.push_back(student);
    if (!saveDatabase()) {
        return jsonError("Unable to save the student record. Please try again.");
    }

    ostringstream response;
    response << "{\n";
    response << "  \"success\": true,\n";
    response << "  \"message\": \"Registration successful.\",\n";
    response << "  \"student\": " << makeJsonSafeStudent(student, false) << "\n";
    response << "}";
    return response.str();
}

string routeApiLogin(const string& requestBody) {
    string username = trim(extractJsonString(requestBody, "username"));
    string password = extractJsonString(requestBody, "password");

    if (username.empty() || password.empty()) {
        return jsonError("Please provide both username and password.");
    }

    Student* student = findStudentByUsername(username);
    if (!student) {
        return jsonError("Username not found.");
    }
    if (student->password != password) {
        return jsonError("Incorrect password.");
    }

    ostringstream response;
    response << "{\n";
    response << "  \"success\": true,\n";
    response << "  \"student\": " << makeJsonSafeStudent(*student, false) << "\n";
    response << "}";
    return response.str();
}

string routeApiStudents() {
    ostringstream response;
    response << "{\n";
    response << "  \"success\": true,\n";
    response << "  \"students\": " << makeStudentListJson() << "\n";
    response << "}";
    return response.str();
}

string routeApiForgetPassword(const string& requestBody) {
    string username = trim(extractJsonString(requestBody, "username"));
    string email = trim(extractJsonString(requestBody, "email"));

    if (username.empty() || email.empty()) {
        return jsonError("Please provide both username and email.");
    }

    Student* student = findStudentByUsername(username);
    if (!student) {
        return jsonError("Username not found.");
    }
    if (student->email != email) {
        return jsonError("Email does not match the username.");
    }

    ostringstream response;
    response << "{\n";
    response << "  \"success\": true,\n";
    response << "  \"message\": \"Your password is: " << escapeJson(student->password) << "\"\n";
    response << "}";
    return response.str();
}

string routeStaticFile(const string& path) {
    string normalized = path;
    if (normalized == "/" || normalized.empty()) {
        normalized = "/" + INDEX_FILE;
    }
    if (normalized.front() == '/') normalized.erase(0, 1);
    if (normalized.find("..") != string::npos) {
        return "";
    }

    string content;
    if (!readFile(normalized, content)) {
        return "";
    }
    return content;
}

int createServerSocket(int port) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) return -1;

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opt), sizeof(opt));

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0) {
        return -1;
    }
    if (listen(serverSocket, 10) < 0) {
        return -1;
    }
    return serverSocket;
}

int main() {
    initializeSockets();

    if (!loadDatabase()) {
        cerr << "Unable to load student database from " << DB_FILE << "\n";
        return EXIT_FAILURE;
    }

    int serverSocket = createServerSocket(SERVER_PORT);
    if (serverSocket < 0) {
        cerr << "Failed to start the server on port " << SERVER_PORT << ".\n";
        cleanupSockets();
        return EXIT_FAILURE;
    }

    cout << "Student portal backend is running at http://localhost:" << SERVER_PORT << "/\n";
    cout << "Open the page in your browser and use the form to register or login.\n";

    while (true) {
        sockaddr_in clientAddress{};
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressLength);
        if (clientSocket < 0) continue;

        string requestLine;
        string requestBody;
        map<string, string> headers;
        if (!receiveAll(clientSocket, requestLine, requestBody, headers)) {
#ifdef _WIN32
            closesocket(clientSocket);
#else
            close(clientSocket);
#endif
            continue;
        }

        istringstream requestStream(requestLine);
        string method, path, httpVersion;
        requestStream >> method >> path >> httpVersion;
        string response;
        string contentType = "application/json; charset=utf-8";

        if (method == "OPTIONS") {
            response = buildResponse("200 OK", contentType, "{\"success\":true}\n");
        } else if (path.rfind("/api/register", 0) == 0 && method == "POST") {
            string result = routeApiRegister(requestBody);
            if (result.find("\"error\"") != string::npos) {
                response = buildResponse("400 Bad Request", contentType, result);
            } else {
                response = buildResponse("200 OK", contentType, result);
            }
        } else if (path.rfind("/api/login", 0) == 0 && method == "POST") {
            string result = routeApiLogin(requestBody);
            if (result.find("\"error\"") != string::npos) {
                response = buildResponse("401 Unauthorized", contentType, result);
            } else {
                response = buildResponse("200 OK", contentType, result);
            }
        } else if (path.rfind("/api/students", 0) == 0 && method == "GET") {
            response = buildResponse("200 OK", contentType, routeApiStudents());
        } else if (path.rfind("/api/forget-password", 0) == 0 && method == "POST") {
            string result = routeApiForgetPassword(requestBody);
            if (result.find("\"error\"") != string::npos) {
                response = buildResponse("400 Bad Request", contentType, result);
            } else {
                response = buildResponse("200 OK", contentType, result);
            }
        } else {
            string content = routeStaticFile(path);
            if (content.empty()) {
                response = buildResponse("404 Not Found", "text/plain; charset=utf-8", "Page not found\n");
            } else {
                if (path.find(".css") != string::npos) contentType = "text/css; charset=utf-8";
                else if (path.find(".js") != string::npos) contentType = "application/javascript; charset=utf-8";
                else if (path.find(".html") != string::npos || path == "/" ) contentType = "text/html; charset=utf-8";
                else if (path.find(".json") != string::npos) contentType = "application/json; charset=utf-8";
                else contentType = "text/plain; charset=utf-8";
                response = buildResponse("200 OK", contentType, content);
            }
        }

        sendAll(clientSocket, response);
#ifdef _WIN32
        closesocket(clientSocket);
#else
        close(clientSocket);
#endif
    }

#ifdef _WIN32
    closesocket(serverSocket);
#else
    close(serverSocket);
#endif
    cleanupSockets();
    return EXIT_SUCCESS;
}

/*
 * ============================================================
 *  HOW THIS PROGRAM WORKS (Easy Explanation)
 * ============================================================
 *
 *  1. STRUCT  - Like a form / card that holds student info
 *               (name, id, cgpa, etc. all in one place)
 *
 *  2. ARRAY   - A row of boxes; each box stores one Student
 *               students[0], students[1], students[2] ...
 *
 *  3. FUNCTIONS - Small tasks broken into named blocks
 *               e.g. registerStudent() just does registration
 *
 *  4. LOOP (for) - Used to go through all students one by one
 *
 *  5. SWITCH  - Cleaner way to handle menu choices
 *
 *  6. STRING  - Stores text like names, usernames, passwords
 *
 *  COMPILE & RUN:
 *     g++ student_system.cpp -o student_system
 *     ./student_system         (Linux/Mac)
 *     student_system.exe       (Windows)
 *
 * ============================================================
 */
