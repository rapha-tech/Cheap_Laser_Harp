/*

Programme de test de la génération du son à partir des commandes reçues en midi

https://caml.music.mcgill.ca/~gary/rtmidi/index.html#compiling

sur linux il faut installer les lib alsa pour developpeur
command pour build sur linux : g++ -Wall -D__LINUX_ALSA__ -o main main.cpp lib/RtMidi.cpp -lasound -lpthread
command pour build sur windows : g++ -Wall -D__WINDOWS_MM__ -o main main.cpp lib/RtMidi.cpp -lwinmm
command pour run : ./main

*/


#include <iostream>
#include <cstdlib>
#include "lib/RtMidi.h"

#define MINIAUDIO_IMPLEMENTATION
#include "lib/miniaudio_io.h"

#define TSF_IMPLEMENTATION
#include "lib/tsf.h"

using namespace std;

#define NOTE_ON_CHAN_O 0x90
#define NOTE_OFF_CHAN_O 0x80
#define INSTRUMENT 0 // usually piano

#define STATUS_BYTE 0
#define DATA_BYTE_O 1
#define DATA_BYTE_1 2


// Holds the global instance pointer
static tsf* g_TinySoundFont;

// A Mutex so we don't call note_on/note_off while rendering audio samples
static ma_mutex g_Mutex;

// Callback function called by the audio thread
static void AudioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	// Render the audio samples in float format
	ma_mutex_lock(&g_Mutex); //get exclusive lock
	tsf_render_float(g_TinySoundFont, (float*)pOutput, (int)frameCount, 0);
	ma_mutex_unlock(&g_Mutex);
}

// This function should be embedded in a try/catch block in case of
// an exception.  It offers the user a choice of MIDI ports to open.
// It returns false if there are no ports available.
bool chooseMidiPort( RtMidiIn *rtmidi );

void midiInCallback( double deltatime, vector<unsigned char> *message, void *userData )
{
    // http://midi.teragonaudio.com/tech/midispec/noteon.htm
    // http://midi.teragonaudio.com/tech/midispec/noteoff.htm
    unsigned int nBytes = message->size();
    if(nBytes == 3)
    {
        if (message->at(STATUS_BYTE) == NOTE_ON_CHAN_O)
        {
            cout << "note_on" << endl;
            ma_mutex_lock(&g_Mutex);
            tsf_note_on(g_TinySoundFont, INSTRUMENT, message->at(DATA_BYTE_O), 1.0f);
            //tsf_note_on(g_TinySoundFont, INSTRUMENT, message->at(DATA_BYTE_O), (float)(message->at(DATA_BYTE_1)/127));
            ma_mutex_unlock(&g_Mutex);
        }
        else if (message->at(STATUS_BYTE) == NOTE_OFF_CHAN_O)
        {
            cout << "note_off" << endl;
            ma_mutex_lock(&g_Mutex);
            tsf_note_off(g_TinySoundFont, INSTRUMENT, message->at(DATA_BYTE_O));
            ma_mutex_unlock(&g_Mutex);
        }
    }

    if (nBytes > 0)
        cout << "stamp = " << deltatime << endl;
}
 
int main()
{
    // set up the audio and soundfont part
    // Define the desired audio output format we request
	ma_device device;
	ma_device_config deviceConfig;
	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format = ma_format_f32;
	deviceConfig.playback.channels = 2;
	deviceConfig.sampleRate = 44100;
	deviceConfig.dataCallback = AudioCallback;

	// Initialize the audio system
	if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS)
	{
		fprintf(stderr, "Could not initialize audio hardware or driver\n");
		return 1;
	}

	// Load the SoundFont from a file
	g_TinySoundFont = tsf_load_filename("florestan-subset.sf2");
	if (!g_TinySoundFont)
	{
		fprintf(stderr, "Could not load SoundFont\n");
		return 1;
	}

	// Set the SoundFont rendering output mode
	tsf_set_output(g_TinySoundFont, TSF_STEREO_INTERLEAVED, (int)deviceConfig.sampleRate, 0);

	// Create the mutex
	ma_mutex_init(&g_Mutex);

	// Start the actual audio playback here
	// The audio thread will begin to call our AudioCallback function
	if (ma_device_start(&device) != MA_SUCCESS)
	{
		fprintf(stderr, "Failed to start playback device.\n");
		ma_device_uninit(&device);
		return 1;
	}

	// print the differents instruments available in the soundfont
	for (int i = 0; i < tsf_get_presetcount(g_TinySoundFont); i++)
	{
		printf("instrument %d : %s\n", i, tsf_get_presetname(g_TinySoundFont, i));
	}


    // set up the midi part
    RtMidiIn *midiin = new RtMidiIn();
    
    // Call function to select port.
    if (chooseMidiPort( midiin ) == false) goto cleanup;
    
    // Set our callback function.  This should be done immediately after
    // opening the port to avoid having incoming messages written to the
    // queue.
    midiin->setCallback(&midiInCallback);
    
    // Don't ignore sysex, timing, or active sensing messages.
    midiin->ignoreTypes(false, false, false);
    
    cout << "\nReading MIDI input ...\n";

    while(1)
	{
	
	}
	
	ma_device_uninit(&device);
    // We could call tsf_close(g_TinySoundFont) and ma_mutex_uninit(&g_Mutex)
	// here to free the memory and resources but we just let the OS clean up
	// because the process ends here.
    
// Clean up
cleanup:
    delete midiin;
    
    return 0;
}

bool chooseMidiPort( RtMidiIn *rtmidi )
{
    string portName;
    unsigned int i = 0, nPorts = rtmidi->getPortCount();
    if (nPorts == 0)
    {
        cout << "No input ports available!" << endl;
        return false;
    }

  
    for (i = 0; i < nPorts; i++)
    {
        portName = rtmidi->getPortName(i);
        cout << "Input port #" << i << ": " << portName << '\n';
    }

    do
    {
        cout << "\nChoose a port number: ";
        cin >> i;
    } while (i >= nPorts);
  

    rtmidi->openPort(i);

    return true;
}