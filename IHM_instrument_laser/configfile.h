#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "lib/yyjson.h"

#include <QString>

class configFile
{
private:
    QString SoundFont_path;
    int instrument_id;
    int volume;
    int port_id;

public:
    configFile(QString&);

    void write(QString&);

    QString get_soundFont_path();
    int get_instr_id();
    int get_volume();
    int get_port_id();

    void set_soundFont_path(QString&);
    void set_instr_id(int);
    void set_volume(int);
    void set_port_id(int);
};

#endif // CONFIGFILE_H
