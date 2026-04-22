const express = require('express');
const fs = require('fs');

const app = express();
app.use(express.json());

let students = [];

// Load data
if (fs.existsSync('students.json')) {
  students = JSON.parse(fs.readFileSync('students.json'));
}

// Register
app.post('/api/register', (req, res) => {
  const { name, username, password, email } = req.body;

  const newUser = {
    id: Date.now(),
    name,
    username,
    password,
    email
  };

  students.push(newUser);
  fs.writeFileSync('students.json', JSON.stringify(students, null, 2));

  res.json({ student: newUser });
});

// Login
app.post('/api/login', (req, res) => {
  const { username, password } = req.body;

  const user = students.find(u => u.username === username && u.password === password);

  if (!user) {
    return res.status(400).json({ message: 'Invalid credentials' });
  }

  res.json({ student: user });
});

// Get all users
app.get('/api/students', (req, res) => {
  res.json({ students });
});

app.listen(8080, () => {
  console.log('Server running on http://localhost:8080');
});