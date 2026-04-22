# 🚀 User Portal

> Full-stack User Management System with Authentication, Admin Panel & Notes 

## Features

✨ **User Management**
- User registration with validation
- Secure login authentication
- Profile viewing and editing
- Account deletion
- Password management

👥 **Admin Panel** (for admin users)
- View all registered users
- Search users by name or email
- User role management
- Post announcements to all users
- User activity tracking

📝 **User Features**
- Personal notes creation and management
- View system announcements
- Edit profile information
- Secure account management

🎨 **UI/UX**
- Modern dark mode interface with gradient accents
- Fully responsive design (mobile, tablet, desktop)
- Smooth animations and transitions
- Intuitive navigation with tabbed interface
- Real-time search functionality

## Screenshots

### Login Page
![User Hub Login](./images/login.png)

### Portal Dashboard
![User Hub Portal - Profile Tab](./images/portal.png)

## Tech Stack

- **Backend:** C++ with HTTP server capability (or Node.js with Express)
- **Frontend:** HTML5, CSS3, JavaScript (Vanilla)
- **Storage:** JSON (persistent file-based database)
- **Architecture:** RESTful API with stateless endpoints

## Project Structure

```
User Portal/
├── student_system.cpp          # C++ HTTP server
├── server.js                   # Node.js Express server (alternative)
├── student_portal.html         # Main frontend application
├── students.json              # User data storage
├── announcements.json         # Announcements storage
└── README.md                  # This file
```

## How It Works

- `student_system.cpp` (or `server.js`) runs a local HTTP server on port 8080
- Serves `student_portal.html` as the main application
- Exposes REST API endpoints for user operations
- Stores all user and announcement data in JSON files
- Admin users (specifically `demo_admin` by default) can access admin features

## Installation & Setup

### Option 1: C++ Backend

**Prerequisites:**
- C++ compiler (GCC/G++ or MSVC)

**Steps:**

1. Compile the backend:
   - With g++:
     ```bash
     g++ student_system.cpp -o student_system.exe
     ```
   - With MSVC:
     ```bash
     cl /EHsc student_system.cpp
     ```

2. Run the server:
   ```bash
   student_system.exe
   ```

3. Open your browser and navigate to:
   ```
   http://localhost:8080/
   ```

### Option 2: Node.js Backend

**Prerequisites:**
- Node.js and npm

**Steps:**

1. Install dependencies:
   ```bash
   npm install
   ```

2. Start the server:
   ```bash
   node server.js
   ```

3. Open your browser and navigate to:
   ```
   http://localhost:8080/
   ```

## API Endpoints

### Authentication
- **POST** `/api/register` - Register a new user
  ```json
  { "name": "John Doe", "username": "john", "password": "pass123", "email": "john@example.com" }
  ```
  
- **POST** `/api/login` - Authenticate user
  ```json
  { "username": "john", "password": "pass123" }
  ```

### User Management
- **POST** `/api/update-profile` - Update user profile (requires current password)
  ```json
  { "id": 12345, "name": "Jane Doe", "email": "jane@example.com", "currentPassword": "pass123", "newPassword": "newpass456" }
  ```
  
- **POST** `/api/delete-account` - Delete user account
  ```json
  { "id": 12345 }
  ```

### Admin Features
- **GET** `/api/students` - Get all registered users (admin only)
  
- **GET** `/api/announcements` - Get all announcements
  
- **POST** `/api/announcements` - Post new announcement (admin only)
  ```json
  { "title": "Important Update", "message": "System maintenance...", "userId": 12345 }
  ```

## User Roles

### Regular User
- Register and manage personal account
- View and edit profile
- Create and manage personal notes
- View announcements
- Change password

### Admin User
- All regular user features
- View all registered users
- Search users
- Post announcements to all users
- View user activity log

**Default Admin:** `demo_admin` (set automatically for this username during registration)

## Usage

### First Time Setup

1. Start the server (see Installation section)
2. Navigate to `http://localhost:8080/`
3. Click "Register" to create a new account
4. To get admin access, register with username `demo_admin`
5. Login with your credentials

### Login & Registration Flow
- Clean, intuitive interface with two tabs: Register and Login
- Form validation with real-time feedback
- Responsive design works on mobile, tablet, and desktop

### Admin Dashboard
- After login, demo_admin sees the complete admin interface
- Dashboard displays all registered users in an organized table
- Search functionality to quickly find users
- Admin can post announcements visible to all users

### As a Regular User

- View your profile after login
- Create personal notes in the "My Notes" tab
- View announcements in the "Announcements & Updates" tab
- Edit profile or change password using the "Edit Profile" button
- Delete account if needed

### As an Admin

- All regular user features
- Access "Registered users" table to view all system users
- Use search to find specific users
- Post announcements visible to all users
- View "User Activity" to track registrations and changes

## Data Storage

- **students.json** - Stores all user profiles, credentials, and roles
- **announcements.json** - Stores system announcements

Both files are automatically created on first run.

## Security Notes

⚠️ **Important:** This is a demonstration project. For production use:
- Implement proper password hashing (bcrypt, scrypt, etc.)
- Use HTTPS/TLS for data encryption
- Add CSRF protection
- Implement rate limiting
- Add input validation and sanitization
- Store sensitive data securely
- Use environment variables for configuration
- Implement session management with tokens (JWT)

## Browser Compatibility

- Chrome/Chromium (latest)
- Firefox (latest)
- Safari (latest)
- Edge (latest)
- Mobile browsers with ES6 support

## 📋 Quick Reference

### User Types & Permissions

| Feature | Regular User | Admin (demo_admin) |
|---------|:----------:|:--:|
| Register & Login | ✅ | ✅ |
| View Profile | ✅ | ✅ |
| Edit Profile | ✅ | ✅ |
| Create Notes | ✅ | ✅ |
| View Announcements | ✅ | ✅ |
| Post Announcements | ❌ | ✅ |
| View All Users | ❌ | ✅ |
| Search Users | ❌ | ✅ |
| View Activity Log | ❌ | ✅ |
| Delete Account | ✅ | ✅ |

### Default Test Accounts

| Username | Password | Role |
|----------|----------|------|
| `demo_admin` | `demo123` | Admin |
| `demo_user` | `demo123` | User |

## Performance Notes

- The frontend communicates with the backend via `fetch()` API
- The app is fully responsive and optimized for small screens
- JSON file I/O is suitable for small to medium user bases
- For larger deployments, consider using a proper database (MongoDB, PostgreSQL, etc.)

## Troubleshooting

**Port 8080 already in use:**
- Change the port in `server.js` or modify the C++ server code
- Or close the application using port 8080

**Module not found (Node.js):**
```bash
npm install express fs path
```

**Access denied when deleting students.json:**
- Close the running server before modifying files
- Ensure write permissions in the project directory

## Future Enhancements

- Email verification for registration
- Password reset functionality
- User avatar/profile pictures
- Email notifications
- Advanced user filtering and sorting
- User activity analytics
- Two-factor authentication
- Role-based access control (RBAC)
- Database integration (MongoDB/PostgreSQL)
- Docker containerization

## License

This project is open source and available for educational and personal use.

## Support

For issues or questions, please review the code comments and API documentation above.
