#include <iostream>
#include <cmath>
#include <unistd.h>
#include <string.h>

#include <jack/jack.h>
using namespace std;

/* A 4th order to filter that will approximate pink noise when 
   given white noise as input
   This filter is based on Julius O. Smith III's design presented 
   int Spectral Audio Signal Processing, published by the Center
   for Computer Research in Music and Acoustics at Stanford in 2011.
   ccrma.stanford.edu/~jos/sasp/sasp.html                           */

typedef jack_default_audio_sample_t sample_t;

jack_client_t *client;
jack_port_t *input_port, *output_port;

//Coefficients for filter
const float B[4] = {0.049922035, -0.095993537, 0.050612699, 0.004408786};
const float A[4] = {1.0, -2.494956002, 2.017265875, -0.5221894};

// history holds the current sample and the previous 3 samples
// these are both circular buffers. current position is used to implement that
float ip_history[4] = {0.0, 0.0, 0.0, 0.0};
float op_history[4] =  {0.0, 0.0, 0.0, 0.0};
int current_position = 0;

int sample_rate_change(){
    cout << "Sample Rate Change. Failure. Exitiing..." << endl;
    return -1;
}

//constrains samples to the range [-1,1]
float clip(float x){
    return (x > 1.0 ? 1.0 : (x < -1.0 ? -1.0 : x));
}

int process(jack_nframes_t nframes, void *arg){
    //find the ip and op buffers
    sample_t *input = (sample_t *) jack_port_get_buffer(input_port, nframes);
    sample_t *output = (sample_t *) jack_port_get_buffer(output_port, nframes);

    for(int i{0}; i < nframes; i++){
        //copy values to history
        ip_history[current_position] = input[i];

        //y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] + b3*x[n-3] - a1*y[n-1] - a2*y[n-2] - a3*y[n-3]
        float b_accumulator = B[0] * ip_history[current_position];
        float a_accumulator = 0.0;
        for(int j{1}; j < 4; j++){
            int target = (current_position - j) % 4;
            b_accumulator += B[j] * ip_history[target];
            a_accumulator += A[j] * op_history[target];
        }
        
        /*float op_sample {0};
        for(int j{0}; j < 4; j++){
            int target = (current_position - j) % 4;

            op_sample += B[j] * ip_history[target];
            op_sample -= (j == 0 ? 0.0 : A[j] * op_history[target]);
        }*/

        //write to the op buffer
        op_history[current_position] = clip(b_accumulator - a_accumulator);
        output[i] = op_history[current_position];

        //cout << output[i] - in

        //increment the index for the circular buffers
        current_position = (current_position + 1) % 4;
    }

    return 0;
}

int main(int argc, char *argv[]){
    if((client = jack_client_open("pink_filter", JackNoStartServer,0))==0){
        cout << "Failed to connect to Jack Server. Exiting..." << endl;
        return 1;
    }

    jack_set_process_callback(client, process, 0);

    input_port = jack_port_register(client, "pinked_in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput,0);
    output_port = jack_port_register(client, "pinked_out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput,0);

    if(jack_activate(client)){
        cout << "cannot activate client. Exiting..." << endl;
        return 1;
    }

    while(1)
        usleep(1);
}

