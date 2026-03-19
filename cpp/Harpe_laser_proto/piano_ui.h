#ifndef PIANO_UI_H
#define PIANO_UI_H

#include <QGraphicsScene>

#include "PianoKey.h"

class Piano_ui
{
private:
    PianoKey** Keys;

public:
    Piano_ui(QGraphicsScene*);
    int* get_note_state();
};

#endif // PIANO_UI_H
