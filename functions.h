#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <omp.h>

#define PORT_NO 3333

struct State {
    int send_receive_mode;
    int file_socket_mode;
};

void check_file_name(char *path)
{
    char *split;
    char *file_name;

    split = strtok(path, "/");

    while(split != NULL) {
        file_name = split;
        split = strtok(NULL, "/");
    }

    if(strcmp(file_name, "chart"))
    {
        fprintf(stderr, "ERROR: file name must be \"chart\". Current name: %s\n", file_name);
        exit(1);
    }
}

int array_contains(const int size, char *arr[], char *word)
{
    int found_flag = 0;

    for(int i = 0; i < size; i++) {
        if(!strcmp(arr[i], word))
        {
            found_flag = 1;
        }
    }

    return found_flag;
}

void print_version_exit(int exit_status)
{
    if(exit_status == 1)
    {
        fprintf(stderr, "ERROR: Invalid arguments\n");
    }
    #pragma omp parallel sections
    {
        #pragma omp section
        {
        printf("Version 1.0\n");
        }
        #pragma omp section
        {
        printf("Copyright (c) 2023 Peter Madai\n");
        }
    }
    exit(exit_status);
}

void print_help_exit(int exit_status)
{
    if(exit_status == 1)
    {
        fprintf(stderr, "ERROR: Invalid arguments\n");
    }
    puts("");
    printf("Usage: chart [OPTIONS]\n");
    printf("\n");
    printf("Options:\n");
    printf("  --help            Show this help message and exit.\n");
    printf("  --version         Show program version number and exit.\n");
    printf("  -send             Set send/receive mode to send.\n");
    printf("  -receive          Set send/receive mode to receive.\n");
    printf("  -file             Set file/socket mode to file.\n");
    printf("  -socket           Set file/socket mode to socket.\n");
    exit(exit_status);
}

int get_char_array_size(char *arr[])
{
    int counter = 0;
    while(strcmp(arr[counter], "\0"))
    {
        counter++;
    }
    return counter;
}

int array_has_duplicate(const int size, char *arr[])
{
    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < size; j++)
        {
            if(strcmp(arr[i], arr[j]) == 0 && i != j)
            {
                return 1;
            }
        }
    }
    return 0;
}

void check_args(int argc, char *argv[])
{
    char *valid_args[] = {"--version", "--help", "-send", "-receive", "-socket", "-file", "\0"};

    if(array_has_duplicate(argc, argv))
    {
        print_help_exit(1);
    }

    for(int i = 1; i < argc; i++)
    {
        // tester
        //printf("%s %d\n", argv[i], array_contains(get_char_array_size(valid_args), valid_args, argv[i]));

        if(!array_contains(get_char_array_size(valid_args), valid_args, argv[i]))
        {
            print_help_exit(1);
        }
    }

    if(array_contains(argc, argv, "--version"))
    {
        if(argc == 2)
        {
            print_version_exit(0);
        }
        else
        {
            print_help_exit(1);
        }
    }
    if(array_contains(argc, argv, "--help"))
    {
        if(argc == 2)
        {
            print_help_exit(0);
        }
        else
        {
            print_help_exit(1);
        }
    }
}

struct State get_working_state(int argc, char* argv[])
{
    struct State state;

    //init default working state
    state.send_receive_mode = 0;  // 0-send 1-receive
    state.file_socket_mode = 0;   // 0-file 1-socket

    if(array_contains(argc, argv, "-receive"))
    {
        if(array_contains(argc, argv, "-send"))
        {
            print_help_exit(1);
        }
        state.send_receive_mode = 1;
    }
    if(array_contains(argc, argv, "-socket"))
    {
        if(array_contains(argc, argv, "-file"))
        {
            print_help_exit(1);
        }
        state.file_socket_mode = 1;
    }

    return state;
}

int max(int a, int b)
{
    if(a > b) return a;
    else return b;
}

void print_int_arr(int size, int* array)
{
    for(int i = 0; i < size; i++)
    {
        printf("%d ", array[i]);
    }
    puts("");
}

int Measurement(int **Values)
{
    srand(time(NULL));
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    int after_quarter = (*timeinfo).tm_min % 15 * 60 + (*timeinfo).tm_sec;
    int generation_size = max(after_quarter, 100);
    *Values = malloc(sizeof(int)*generation_size);

    float random;
    int x_pos = 0;

    for(int i = 0; i < generation_size; i++)
    {
        (*Values)[i] = x_pos;

        random = (float)rand() / RAND_MAX * 100;
        if(random <= 42.8571) x_pos += 1;
        else if(42.8571 < random && random <= 42.8571+(float)11/31*100) x_pos -= 1;
        //printf("DEBUG: %f %f %d\n", random, 42.8571+(float)11/31*100, x_pos);
    }

    printf("\n# Measurements: size = %d\n\n", generation_size);
    return generation_size;
}

void write_int(char *Values, int value)
{
    Values[0] = value;
    Values[1] = value >> 8;
    Values[2] = value >> 16;
    Values[3] = value >> 24;
}

int int_pow(int num, int power)
{
    int result = 1;

    for(int i = 0; i < power; i++)
    {
        result *= num;
    }

    return result;
}

void BMPcreator(int *Values, int NumValues)
{
    //*Debug: set custom image width
    //NumValues = 1000;

    int padding = 0;
    if(NumValues % 32 != 0)
    {
        padding = 32 - (NumValues % 32);
    }

    int image_width = NumValues;
    const int header_size_bytes = 62;
    int arr_size_bytes = (image_width + padding) * image_width / 8;
    int file_size_bytes = arr_size_bytes + header_size_bytes;

    //printf("\n### BMPcretor Debug ###\nPadding= %d\nImage width= %d\nArray size (bytes)= %d\nFile size (bytes) = %d\n", padding, image_width, arr_size_bytes, file_size_bytes);

    char *bmp = malloc(file_size_bytes);

    //*Zero the memory
    for(int i = 0; i < file_size_bytes; i++)
    {
        bmp[i] = 0x00;
    }

    bmp[0] = 'B';
    bmp[1] = 'M';
    write_int(&bmp[2], file_size_bytes);
    write_int(&bmp[6], 0); //*Unused
    write_int(&bmp[10], 62); //*Pixel array offset

    write_int(&bmp[14], 40); //*DIB header size
    write_int(&bmp[18], image_width);
    write_int(&bmp[22], image_width);
    bmp[26] = 0x01; //*Planes
    bmp[27] = 0x00; //*Planes
    bmp[28] = 0x01; //*Bits per pixel
    bmp[29] = 0x00; //*Bits per pixel
    write_int(&bmp[30], 0); //*Compression
    write_int(&bmp[34], 0); //*Image size
    write_int(&bmp[38], 3937); //*Horizontal pixel per meter
    write_int(&bmp[42], 3937); //*Vertical pixel per meter
    write_int(&bmp[46], 0); //*Colors in palette
    write_int(&bmp[50], 0); //*Used palette colors

    bmp[54] = 0xCC; //*Color 0 Blue
    bmp[55] = 0x00; //*Color 0 Green
    bmp[56] = 0xCC; //*Color 0 Red
    bmp[57] = 0xFF; //*Color 0 Alpha
    bmp[58] = 0x00; //*Color 1 Blue
    bmp[59] = 0xFF; //*Color 1 Green
    bmp[60] = 0x00; //*Color 1 Red
    bmp[61] = 0xFF; //*Color 1 Alpha

    int line_bit_count = image_width + padding;
    int center_line = image_width / 2 + 1; // + 1 mert pl 5 közepe 3 viszont 5/2 az 2

    for(int i = 0; i < NumValues; i++)
    {
        // a byteon belüli írás helye
        char mask = (char)int_pow(2, 7 - (i % 8));

        if(abs(Values[i]) > image_width / 2)
        {
            if(Values[i] > 0) Values[i] = (image_width / 2)-1;
            else Values[i] = (-image_width / 2)-1;
        }

        // sor abszolút száma
        int current_line_offset = center_line + Values[i];
        //a sort kezdő index
        int offset_line_index = ((image_width + padding)/8) * current_line_offset;
        // a soron beluli index
        int current_byte_index = offset_line_index + (i / 8);
        bmp[62 + current_byte_index] |= mask;

        //* vonal vastagítás innentől
        current_line_offset = center_line + Values[i] + 1;
        offset_line_index = ((image_width + padding)/8) * current_line_offset;
        current_byte_index = offset_line_index + (i / 8);
        bmp[62 + current_byte_index] |= mask;

        current_line_offset = center_line + Values[i] - 1;
        offset_line_index = ((image_width + padding)/8) * current_line_offset;
        current_byte_index = offset_line_index + (i / 8);
        bmp[62 + current_byte_index] |= mask;
    }

    int f = open("output.bmp", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR);
    write(f, bmp, file_size_bytes);
    printf("BMP file created!\n\n");
    close(f);
    free(bmp);

}

int FindPID()
{
    DIR *d;
    struct dirent *entry;
    FILE *f;
    char line_buf[100];

    int Pid = -1;
    int result_pid = -1;

    d=opendir("/proc/");

    while((entry=readdir(d))!=NULL)
    {
        if('0' <= (*entry).d_name[0] && (*entry).d_name[0] <= '9')
        {
            char tmp_path[20] = "/proc/";
            int tmp_index = 0;

            // concatenate directory name to /proc/ mert máshogy a világért nem akar menni
            // se strcat se sprintf
            while((*entry).d_name[tmp_index] != '\0')
            {
                tmp_path[6+tmp_index] = (*entry).d_name[tmp_index];
                tmp_index++;
            }
            tmp_path[6+tmp_index] = '\0';

            f = fopen(strcat(tmp_path,"/status"), "r");
            if(f != NULL)
            {
                fgets(line_buf, 100, f);
                char name_buf[100];
                sscanf(line_buf, "Name:\t%s\n", name_buf);
                if(strcmp(name_buf, "chart") == 0)
                {
                    printf("\n# process_name: %s\n", name_buf);
                    for(int i = 0; i < 5; i++)
                    {
                        fgets(line_buf, 100, f);
                    }
                    printf("# Pid_line: %s", line_buf);
                    sscanf(line_buf, "Pid:\t%d\n", &Pid);
                    printf("# pid: %d\n", Pid);

                    if(Pid != getpid())
                    {
                        result_pid = Pid;
                    }
                }
            }
            fclose(f);
        }
    }
    closedir(d);

    return result_pid;
}

void SendViaFile(int *Values, int NumValues)
{
    printf("\n# Send via file\n");
    char file_path[256];
    //printf("\n# Debug: file_path_size: %d\n", sizeof(file_path));

    //* Get the path of current user's home directory
    snprintf(file_path, sizeof(file_path), "%s/Measurement.txt", getenv("HOME"));

    FILE *f = fopen(file_path, "w");

    for(int i = 0; i < NumValues; i++)
    {
        fprintf(f, "%d\n", Values[i]);
    }
    fclose(f);
    printf("\nFile is ready.\n");

    if(FindPID() == -1)
    {
        fprintf(stderr, "ERROR: Could not find other process in receiving mode\n");
        exit(6);
    }
    //else
    printf("Notifying receiver.\n");
    kill(FindPID(), SIGUSR1);
}

void ReceiveViaFile(int sig)
{
    printf("\n# Receive via file\n\n");
    char file_path[256];

    //* Get the path of current user's home directory
    snprintf(file_path, sizeof(file_path), "%s/Measurement.txt", getenv("HOME"));

    const int CHUNK_SIZE = 128;
    int full_size = CHUNK_SIZE;
    int useful_size = 0;

    int *arr = malloc(sizeof(int) * full_size);
    printf("# Allocated memory. size: %d * 4bytes\n", full_size);
    FILE *f = fopen(file_path, "r");
    char line[1024];

    while(fgets(line, 1024, f) != NULL)
    {
        int tmp_value = 0;
        sscanf(line, "%d\n", &tmp_value);

        useful_size += 1;
        if (useful_size >= full_size)
        {
            // ha betelt a lefoglalt terület akkor a chunk mérettel növeli
            arr = realloc(arr, (sizeof(int) * full_size) + (sizeof(int)* CHUNK_SIZE));
            full_size += CHUNK_SIZE;
            printf("# Reallocated memory. size: %d * 4bytes\n", full_size);
        }

        arr[useful_size-1] = tmp_value;
    }

    printf("# Starting BMPcreator with size= %d\n", useful_size);
    BMPcreator(arr, useful_size);
    free(arr);
    printf("# Unallocated memory\n");
    printf("# Receiving procedure finished\n");
    printf("\n#####################################################\n");
}

void SignalHandler(int sig)
{
    if ( sig == SIGINT )
    {
        printf("Goodbye (͡° ͜ʖ ͡°)\n\n");
        exit(0);
    }
    if ( sig == SIGUSR1 )
    {
        fprintf(stderr, "ERROR: Sending through file is not available.\n");
    }
    if( sig == SIGALRM )
    {
        fprintf(stderr, "ERROR: The server is not responding.\n");
        exit(7);
    }
}

void SendViaSocket(int *Values, int NumValues)
{
    /************************ Declarations **********************/
    int s;                            // socket ID
    int bytes;                        // received/sent bytes
    int flag;                         // transmission flag
    char on;                          // sockopt option
    //char buffer[BUFSIZE];             // datagram buffer area
    unsigned int server_size;         // length of the sockaddr_in server
    struct sockaddr_in server;        // address of server
    /************************ Initialization ********************/
    on   = 1;
    flag = 0;
    server.sin_family      = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port        = htons(PORT_NO);
    server_size = sizeof server;

    /************************ Creating socket *******************/
    s = socket(AF_INET, SOCK_DGRAM, 0 );
    if ( s < 0 ) {
        fprintf(stderr, "ERROR: Socket creation error.\n");
        exit(2);
        }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);
    /************************ Sending data **********************/
    int NumValuesLong = (long)NumValues;
    bytes = sendto(s, &NumValuesLong, sizeof(NumValuesLong), flag, (struct sockaddr *) &server, server_size);
    signal(SIGALRM, SignalHandler);
    alarm(1);
    if ( bytes <= 0 ) {
        fprintf(stderr, "ERROR: Sending error.\n");
        exit(3);
        }
    printf ("# %i bytes have been sent to server with value: %d.\n", bytes, NumValuesLong);
    /************************ Receive data **********************/
    int received;
    bytes = recvfrom(s, &received, sizeof(received), flag, (struct sockaddr *) &server, &server_size);
    alarm(0);
    if ( bytes < 0 ) {
        fprintf(stderr, "ERROR: Receiving error.\n");
        exit(4);
        }
    printf("# Server's (%s:%d) acknowledgement: %d\n",
            inet_ntoa(server.sin_addr), ntohs(server.sin_port), received);

    if(received != NumValuesLong)
    {
        fprintf(stderr, "ERROR: Handshake missmatch! Connection aborted.\n");
        exit(5);
    }
    //else
    printf("# Handshake match! Connection successful.\n");

    /*********************** Sending useful data ******************/
    bytes = sendto(s, Values, NumValuesLong * sizeof(int), flag, (struct sockaddr *) &server, server_size);
    if ( bytes <= 0 ) {
        fprintf(stderr, "ERROR: Sending error.\n");
        exit(3);
    }
    printf ("# %i bytes have been sent to server.\n", bytes);

    /***************** Receive array size for checking **********************/

    bytes = recvfrom(s, &received, sizeof(received), flag, (struct sockaddr *) &server, &server_size);
    if ( bytes < 0 ) {
        fprintf(stderr, "ERROR: Receiving error.\n");
        exit(4);
        }
    printf("# Server's (%s:%d) (size in bytes) response: %d\n",
            inet_ntoa(server.sin_addr), ntohs(server.sin_port), received);

    if(received != NumValues * sizeof(int))
    {
        fprintf(stderr, "ERROR: Size check response (%d) did not match the original size (%d).\n", received, NumValues);
        exit(5);
    }
    //else
    printf("# Size check response match! Data transmission successful.\n");
    free(Values);
}

void ReceiveViaSocket()
{
    /************************ Declarations **********************/
    int s;
    int bytes;                        // received/sent bytes
    int err;                          // error code
    int flag;                         // transmission flag
    char on;                          // sockopt option
    //char buffer[BUFSIZE];             // datagram buffer area
    unsigned int server_size;         // length of the sockaddr_in server
    unsigned int client_size;         // length of the sockaddr_in client
    struct sockaddr_in server;        // address of server
    struct sockaddr_in client;        // address of client
    /************************ Initialization ********************/
    on   = 1;
    flag = 0;
    server.sin_family      = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port        = htons(PORT_NO);
    server_size = sizeof server;
    client_size = sizeof client;
    /************************ Creating socket *******************/
    s = socket(AF_INET, SOCK_DGRAM, 0 );
    if ( s < 0 ) {
        fprintf(stderr, "ERROR: Socket creation error.\n");
        exit(2);
        }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    /************************ Binding socket ********************/
    err = bind(s, (struct sockaddr *) &server, server_size);
    if ( err < 0 ) {
        fprintf(stderr,"ERROR: Binding error.\n");
        exit(3);
        }

    while(1){ // Continuous server operation
        /************************ Receive data **********************/
        int numValues;
        printf("\n Waiting for a message...\n");
        bytes = recvfrom(s, &numValues, sizeof(numValues), flag, (struct sockaddr *) &client, &client_size );
        if ( bytes < 0 ) {
            fprintf(stderr, "ERROR: Receiving error.\n");
            exit(4);
        }
        printf (" %d bytes have been received from the client (%s:%d).\n Client's message:  %d\n",
                bytes, inet_ntoa(client.sin_addr), ntohs(client.sin_port), numValues);

        /************************ Sending response **********************/
        bytes = sendto(s, &numValues, sizeof(numValues), flag, (struct sockaddr *) &client, client_size);
        if ( bytes <= 0 ) {
            fprintf(stderr, "ERROR: Sending error.\n");
            exit(3);
        }
        printf("# Acknowledgement have been sent to client.\n");

        /******************* Acquiring measurement data ********************/
        int *arr = malloc(numValues * sizeof(int));
        bytes = recvfrom(s, arr, numValues * sizeof(int), flag, (struct sockaddr *) &client, &client_size );
        if ( bytes < 0 ) {
            fprintf(stderr, "ERROR: Receiving error.\n");
            exit(4);
        }
        printf (" %d bytes have been received from the client (%s:%d).\n",
                bytes, inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        //print_int_arr(numValues, arr);

        /************************ Sending size check **********************/
        int tmp = numValues * sizeof(int);
        bytes = sendto(s, &tmp, sizeof(int), flag, (struct sockaddr *) &client, client_size);
        if ( bytes <= 0 ) {
            fprintf(stderr, "ERROR: Sending error.\n");
        }
        printf("# Data size in bytes have been sent to client with value: %ld bytes.\n", numValues * sizeof(int));

        BMPcreator(arr, numValues);
        free(arr);
    }
}
