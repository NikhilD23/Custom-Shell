# Custom-Shell 

--> Built-in Commands (Custom Implemented)
    cd <dir> – Change directory
    history – Show command history
    clear – Clears the terminal screen
    mkdir <dir> – Creates a new directory
    rmdir <dir> – Removes an empty directory

--> System Commands (Handled by execvp)
    Any command available in Linux, like:
    ls, pwd, echo, ps, grep, find, cat, etc.


--> Special Features
    Piping (|) → Example: ls -l | grep cpp
    Output Redirection (>) → Example: ls -l > output.txt
    Runs any external command like /bin/date, /bin/uname, etc.
