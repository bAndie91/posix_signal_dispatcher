# posix_signal_dispatcher

Simple unix-y command wrapper which translates received signals to another signal and re-emit to the wrapped process

# Usage

```
sigdispatch [SIG_A1:[SIG_B1] [SIG_A2:[SIG_B2] [...]]] -- <COMMAND> [ARGUMENTS]
```

First arguments in `SIG_A:SIG_B` format are the signal translation definitions.
After double dash `--` is the command you want to run and its arguments.

Whenever `sigdispatch` receives `SIG_A` it sends `SIG_B` to child process started by COMMAND.
Signals can be written as numbers, symbolic name, or name with "SIG" prefix.
Omitting `SIG_B` makes `SIG_A` ignored, i.e. not relayed to child process.
By default all signals map to itself and being relayed to child process, of course up to the system
limitations, i.e. you can not relay SIGSTOP since you can not even catch it.

# Examples

* `SIGINT:SIGTERM` - receive SIGINT, emit SIGTERM
* `INT:SIGTERM` - same
* `INT:TERM` - same
* `SIGINT:` - receive SIGINT and ignore it, not dispatch
* `INT:15` - signal numbers are also okay

# issues
Please submit issues via PR to some file `issue/TITLE.txt`.
