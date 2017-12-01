# tftp-client
TFTP client written in C. Project developed for learning purposes in Networks and Services Architecture subject of Software Engineering degree at University of Valladolid.

## Content
- [Practice statement](/tftp-client-statement.pdf) - TFTP client development requirements. (Spanish)

## Development

### Requirements
- [Docker](https://www.docker.com/) running on your machine.

### Installation
```bash
# Clone repository.
git clone https://gitlab.com/jorgesanz/tftp-client
cd tftp-client
# Run Ubuntu container sharing repository folder.
docker run --name=ubuntu-tftp-client -it -v $(PWD):/home ubuntu

# Inside the container: get the C compiler tools.
apt-get update
apt-get install build-essential
```

For the next times, having the container already on my machine, how to deploy local environment?
```bash
docker start ubuntu-tftp-client
docker exec -it ubuntu-tftp-client bash
```

### Compilation
```bash
# Inside the container
cd /home
gcc -Wall -o client.out tftp-Sanz-Perez.c
```

### Execution
```bash
client.out server-ip {-r|-w} file [-v]
```

## Deployment
### Remote Linux Virtual Machine via SSH
This C program should be executed in a provided Slackware Linux machine, so one option to send the source files to there is:
```bash
# Transfer file to remote machine
scp -P <port-number> tftp-Sanz-Perez.c youruser@your.machine.address:/destination/folder
```
Then, you can access to your remote machine via SSH and execute the TFTP client from there.

// TODO Use this to avoid password prompt every single time: http://www.linuxproblem.org/art_9.html
