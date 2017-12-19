# tftp-client

TFTP client written in C. Project developed for learning purposes in Networks and Services Architecture subject of Software Engineering degree at University of Valladolid.

## Content

- [Practice statement](/tftp-client-statement.pdf) - TFTP client development requirements. (Spanish)
- [tftp-Sanz-Perez.c](tftp-Sanz-Perez.c) - TFTP client implemented.

## Development

### Requirements

- [GCC](https://gcc.gnu.org), the GNU Compiler Collection.
- TFTP server running ([RFC 1350](https://tools.ietf.org/html/rfc1350)).

### Installation

```bash
# Clone repository.
git clone https://gitlab.com/jorgesanz/tftp-client
cd tftp-client
```

### Compilation

```bash
gcc -Wall -o client.out tftp-Sanz-Perez.c
```

### Execution

- `-r`: read file mode.
- `-w`: write file mode.
- `-v`: tracing mode.

```bash
./client.out server-ip {-r|-w} file [-v]
```

## Deployment

### Remote Linux Virtual Machine via SSH

This C program should be executed in a provided Slackware Linux machine, so one option to send the source files to there is:

```bash
# Transfer file to remote machine
scp -P <port-number> tftp-Sanz-Perez.c youruser@your.machine.address:/destination/folder
```

Then, you can access to your remote machine via SSH and execute the TFTP client from there.
