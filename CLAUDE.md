# MCT2032 - Mega Cyber Tool 2032
## Claude Code Project Instructions

### 🎯 Project Overview
You are working on MCT2032, a portable cybersecurity toolkit using the Waveshare ESP32-S3-LCD-1.47 development board. This project consists of ESP32 firmware and a Python admin console that communicate via Bluetooth LE.

### 🛠️ Required MCP Tools
**ALWAYS use these tools in order of preference:**
1. **Dart** - Primary task management (REQUIRED for all task operations)
2. **Context7** - Latest documentation for software packages
3. **Brave Search** - Backup for documentation and current information
4. **Playwright** - Browser automation and testing
5. **Memory** - Project state and context management

### 📋 Dart Task Management Protocol

**CRITICAL: All development work MUST be tracked in Dart**

#### Task Workflow
1. **Before Starting Work:**
   ```
   - Use dart.get_task to retrieve task details
   - Add comment: "Starting work on [specific feature/fix]"
   - Update status to "Doing"
   ```

2. **During Development:**
   ```
   - Add progress comments every significant milestone
   - Update task description with implementation details
   - Log any blockers or dependencies discovered
   ```

3. **Upon Completion:**
   ```
   - Add detailed completion comment with:
     - What was implemented
     - Any deviations from original plan
     - Testing performed
     - Next steps or related tasks
   - Update status to "Done"
   ```

#### Dart Locations
- **Workspace**: Personal
- **Dartboard**: Personal/MCT2032
- **Docs Folder**: Personal/docs-MCT2032
- **Task Tags**: Use appropriate phase tags (Phase 1-5) and technical tags

### 🎨 UI Design Requirements

**Technology Stack:**
- **Component Library**: shadcn/ui (https://ui.shadcn.com/)
- **Theme**: Dark hacker aesthetic
- **Color Scheme**:
  - Background: Deep dark blue (#0a0e27, #050816)
  - Primary: Purple gradients (#8b5cf6 to #7c3aed)
  - Borders: Purple gradient borders (2px)
  - Headers: Purple gradient text or backgrounds
  - Buttons: Purple gradient with glow effects on hover

**Example Component Styling:**
```tsx
// Button with purple gradient
<Button className="bg-gradient-to-r from-purple-600 to-purple-700 
                   hover:from-purple-700 hover:to-purple-800 
                   border-2 border-purple-500/50 
                   shadow-lg shadow-purple-500/25
                   hover:shadow-purple-500/40 
                   transition-all duration-200">
  Scan Networks
</Button>

// Card with gradient border
<Card className="bg-slate-900/90 backdrop-blur-sm 
                 border-2 border-transparent 
                 bg-gradient-to-r from-purple-500/20 to-blue-500/20 
                 p-[2px]">
  <div className="bg-slate-900 rounded-lg p-6">
    {/* Content */}
  </div>
</Card>
```

### 💻 Development Guidelines

#### Project Structure
```
mct2032/
├── mct2032-firmware/     # ESP32 firmware (PlatformIO)
├── mct2032-admin/        # Python admin console
├── docs/                 # Documentation
├── .claude/              # Claude Code configuration
└── logs/                 # Development logs
```

#### Code Standards
1. **ESP32 Firmware (C++)**:
   - Use PlatformIO
   - Follow ESP-IDF conventions
   - LVGL for UI on circular display
   - JSON for BLE communication

2. **Admin Console (Python)**:
   - Python 3.8+ with type hints
   - Async/await for BLE operations
   - Tkinter base with custom dark theme
   - shadcn/ui inspired components

#### Testing Requirements
- Use Playwright for admin console UI testing
- Unit tests for all core functions
- Integration tests for BLE communication
- Document test results in Dart tasks

### 📝 Documentation Protocol

1. **When viewing docs in Dart:**
   - Use dart.get_doc to read documents
   - Add comments for significant updates
   - Update related tasks when docs change

2. **Code Documentation:**
   - Comprehensive docstrings
   - Implementation examples
   - API documentation for all commands

### 🔧 MCP Tool Usage Examples

#### Context7 for Documentation
```
When implementing new features, ALWAYS check latest docs:
1. resolve-library-id: "esp-idf" 
2. get-library-docs: Use returned ID for ESP32 documentation
3. For Python libs: Check bleak, tkinter, asyncio docs
```

#### Brave Search Backup
```
Use when Context7 doesn't have info:
- "ESP32-S3 promiscuous mode implementation"
- "LVGL circular display mask"
- "BLE GATT characteristic notify Python"
```

#### Memory Management
```
Store in memory:
- Current task status
- Implementation decisions
- Test results
- Bug tracking
- Progress milestones
```

### 📊 Logging Requirements

**Maintain development log in `logs/dev_log.md`:**
```markdown
## [Date] - [Task ID] - [Task Title]
### Started: [Time]
- Initial state: [Description]
- Approach: [Implementation plan]

### Progress:
- [Time]: [What was done]
- [Time]: [Blockers/Issues]

### Completed: [Time]
- Final implementation: [Description]
- Tests passed: [List]
- Next steps: [Related tasks]
```

### 🚀 Quick Start Commands

```bash
# Start development on a task
1. Check Dart for assigned tasks
2. Update task to "Doing"
3. Clone repo and set up environment
4. Implement feature following task description
5. Test thoroughly
6. Update task with results and set to "Done"

# Common workflows
- WiFi Scanner: Phase 1.2 (Task ID: e5Jr8IgncC6n)
- BLE Setup: Phase 1.3 (Task ID: HaIjuoJmgEoo)
- Admin Console: Phase 2.1 (Task ID: fDAtFYXJfu0t)
```

### ⚠️ Critical Reminders

1. **NEVER** skip Dart task updates
2. **ALWAYS** use Context7 before web searching
3. **DOCUMENT** all design decisions in tasks
4. **TEST** with Playwright for UI components
5. **MAINTAIN** consistent dark hacker theme
6. **LOG** all development activities

### 🔐 Security Considerations

- This is a security research tool
- Always include legal disclaimers
- Ensure responsible use messaging
- No active attack implementations
- Focus on detection and analysis

### 📞 Communication Style

When updating Dart tasks or logs:
- Be specific and technical
- Include code snippets
- Reference file paths
- Document decision rationale
- Link related tasks/docs

Remember: The goal is to create a professional, powerful cybersecurity tool with a stunning dark-themed interface that looks like it belongs in a hacker's toolkit!