#include <iostream>
#include <cmath>
#include <random>
#include <unistd.h>
#include <string.h>

#include <jack/jack.h>
using namespace std;

/* A simple jack client that generates white noise to use during testing */

//i'm too lazy to type this over and over
typedef jack_default_audio_sample_t sample_t;

jack_client_t *client;       
jack_port_t *output_port;

//This isn't really needed, but if you change the sample rate things will get wierd
int sample_rate_change(){
    cout << "Sample Rate Change Failure" << endl;
    return -1;
}

//fn to run on each buffer
int process(jack_nframes_t nframes, void*arg){
    //find the hardware buffer
    sample_t *buffer = (sample_t *) jack_port_get_buffer(output_port, nframes);

    //write a random value to each sample in the buffer
    for(int i{0}; i < nframes; i++)
        buffer[i] = (sample_t) ((rand() / (float) RAND_MAX) * 2) - 1;

    //we need to return an int. (1 would signal error)
    return 0;
}

int main(int argc, char *argv[]){
    //try to open a client
    if((client = jack_client_open("white_noise", JackNoStartServer,0))==0){
        cout << "Failed to connect to Jack server" << endl;
        return 1;
    }

    //tell jack to run 'process' on each buffer
    jack_set_process_callback(client, process, 0);

    //setup our output
    output_port = jack_port_register(client, "white_noise", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput,0);

    //start the client
    if(jack_activate(client)){
        cout << "cannot activate client" << endl;
        return 1;
    }

    //wait forever
    while(1)
        usleep(1);
}