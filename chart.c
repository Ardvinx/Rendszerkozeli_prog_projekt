#include "functions.h"

int main(int argc, char* argv[])
{
    signal(SIGINT, SignalHandler);
    signal(SIGUSR1, SignalHandler);

    check_file_name(argv[0]);
    check_args(argc, argv);

    struct State state = get_working_state(argc, argv);

    printf("send/receive=%d file/socket=%d\n", state.send_receive_mode, state.file_socket_mode);

    //* Sending mode
    if(state.send_receive_mode == 0)
    {
        int *measurement_arr = NULL;
        int mesurement_arr_size = Measurement(&measurement_arr);

        //* Sending through file
        if(state.file_socket_mode == 0)
        {
            //print_int_arr(mesurement_arr_size, measurement_arr);
            SendViaFile(measurement_arr, mesurement_arr_size);
            free(measurement_arr);
            exit(0);
        }

        //* Sending through UDP
        if(state.file_socket_mode == 1)
        {
            SendViaSocket(measurement_arr, mesurement_arr_size);
        }
    }

    //* Receiving mode
    if(state.send_receive_mode == 1)
    {
        //* Receiving through file
        if(state.file_socket_mode == 0)
        {
            printf("\n# Waiting for signal from sender\n");
            signal(SIGUSR1, ReceiveViaFile);
            while(1){
                pause();
            }
        }

        //* Receiving through UDP
        if(state.file_socket_mode == 1)
        {
            printf("\n# Waiting for connections.\n");
            ReceiveViaSocket();
        }
    }

    return 0;
}