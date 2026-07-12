import unreal, time
unreal.SystemLibrary.execute_console_command(None, "LiveCoding 0")
print("NEXUS: LiveCoding off")
unreal.SystemLibrary.execute_console_command(None, "LiveCoding 1")
print("NEXUS: LiveCoding on")
unreal.SystemLibrary.execute_console_command(None, "LiveCoding.Compile")
print("NEXUS: compile re-requested")