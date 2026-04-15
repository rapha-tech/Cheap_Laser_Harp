#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "lib/yyjson.h"

class configFile
{
private:
    char* SoundFont_path;
    int instrument_id;
    int volume;
    int port_id;

public:
    configFile(char*);

    void write(char*);
    char* get_soundFont_path();
    int get_instr_id();
    int get_volume();
    int get_port_id();
};

#endif // CONFIGFILE_H
