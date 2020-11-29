#include <iostream>
#include <cmath>
#include <unistd.h>
#include <string.h>

#include <jack/jack.h>
using namespace std;

//A jack client that outputs the difference between to input signals

typedef jack_default_audio_sample_t sample_t;

jack_client_t *client;
jack_port_t *input_port_1, *input_port_2, *output_port;

float clip(float x){
    return (x > 1.0 ? 1.0 : (x < -1.0 ? -1.0: x));
}

int process(jack_nframes_t nframes, void *arg){
    sample_t *input_1 = (sample_t *) jack_port_get_buffer(input_port_1, nframes);
    sample_t *input_2 = (sample_t *) jack_port_get_buffer(input_port_2, nframes);
    sample_t *output = (sample_t *) jack_port_get_buffer(output_port, nframes);

    for(int i{0}; i < nframes; i++){
        output[i] = input_1[i] - input_2[i];
    }

    return 0;
}

int main(int argc, char *argv[]){
    if((client = jack_client_open("difference", JackNoStartServer,0))==0){
        cout << "Failed to connect to Jack Server. Exiting..." << endl;
        return 1;
    }

    jack_set_process_callback(client, process, 0);

    input_port_1 = jack_port_register(client, "input_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput,0);
    input_port_2 = jack_port_register(client, "input_2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput,0);
    output_port = jack_port_register(client, "diff_out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput,0);

    if(jack_activate(client)){
        cout << "cannot activate client. Exiting..." << endl;
        return 1;
    }

    while(1)
        usleep(1);
}
