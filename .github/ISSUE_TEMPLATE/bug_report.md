---
name: "🐛 Bug Report"
about: Report an interpreter crash, syntax error, or bad state evaluation in Swiq
title: "[BUG]: "
labels: bug
assignees: ''
---

## 💥 Describe the Bug
A clear and concise description of what the runtime error is.

## 💻 Minimal Reproducible Script
Provide a minimal `.swiq` script file that triggers the bug:
`swiq
// Paste your code here
`

## 📋 Exact Error Output
Copy and paste the exact C++ exception string thrown by the interpreter:
`text
// Paste interpreter error output here
`

## 🌐 Environment Details
- **Host OS:** (e.g., Linux, Windows, Android/Termux)
- **C++ Compiler & Version:** (e.g., g++ 13, clang++ 17)
- **Swiq Build Version:** (e.g., Swiq.__ENV__.__VERSION__.__BUILD_NUMBER__)

## 🎯 Additional Context
Any extra details, such as whether the variable was previously `archived` or executed inside an isolated closure capture block.
