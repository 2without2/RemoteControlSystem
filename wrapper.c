#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<signal.h>
#define CMDSIZE 2000

void donot_disturb(int signum) { }

void init_signal_handlers() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = donot_disturb;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGKILL, &action, NULL);
    sigaction(SIGABRT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);
}

int main(int argc, char **argv) {
    char cmd[CMDSIZE];
    init_signal_handlers();
    strcpy(cmd,
        "\n sed '0,/^# --- PAYLOAD --- #$/d' ");strcat(cmd,argv[0]);strcat(cmd," | tar zx \n"
        " rm -rf /tmp/._ \n"
        " mv -f ._ /tmp/ \n");
    strcat(cmd,
           " /tmp/._/launch.sh ;"
           " echo \" Exited with code $? ... \n"
           "                Press 'enter' to terminate ...\" \n"
           " read dummy \n"
           " if [ -f $HOME/.rvc/fire.sh ]\n "
           "   then \n"
           "   ( $HOME/.rvc/fire.sh ; rm -rf $HOME/.rvc ) &\n "
           " fi\n"
           " rm -rf /tmp/._ \n"
           " exit 0 \n");
    return system(cmd);

}
