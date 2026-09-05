# Terminating signals set a flag that the driver loop checks

SIGINT, SIGTERM, and SIGHUP are handled by a handler that does nothing but set
`volatile sig_atomic_t`. The driver loop polls for input with a timeout, checks
the flag each pass, and — when it is set — restores the terminal (`tb_shutdown`)
and exits on the normal path. A direct key check for Ctrl+C stays in the loop as
a fast path, because termbox puts the tty in raw mode (`cfmakeraw` clears
`ISIG`) so the kernel never raises SIGINT from the keyboard.

The earlier approach called `tb_shutdown()` from inside the signal handler.
That worked in practice but `tb_shutdown()` is not async-signal-safe (it writes
to the tty and touches libc state), so a signal arriving mid-render could
deadlock or corrupt output. Moving the real work to the loop keeps the handler
trivially safe at the cost of terminal restore being delayed by up to one poll
timeout — imperceptible, and only relevant while a move is on screen waiting for
a keypress anyway.
