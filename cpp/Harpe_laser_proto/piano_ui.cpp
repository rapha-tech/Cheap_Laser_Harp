#include "piano_ui.h"

#define N_OCTAVES 1

#define N_NOTES_OCTAVE 12
#define N_NOTES_BLANCHES 7
int layout[N_NOTES_OCTAVE] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};

Piano_ui::Piano_ui(QGraphicsScene* scene_piano)
{
    Keys = new PianoKey*[N_OCTAVES*N_NOTES_OCTAVE];

    int width_key_white = 52;
    int position = 0;

    // first we print the white keys
    for(int i = 0; i<N_NOTES_OCTAVE; i++)
    {
        if(layout[i] == 0)
        {
            //pRects[i] = scene_piano->addRect(QRectF(position * (width_key_white + 1), 0, width_key_white, 200), QPen(Qt::NoPen), QBrush(Qt::white)); //QPen for border, QBrush for fill
            Keys[i] = new PianoKey(
                QRectF(position * (width_key_white + 1), 0, width_key_white, 200),
                PianoKey::Type::White, i);  // i = note index
            scene_piano->addItem(Keys[i]);
            position++;
        }
    }

    int width_key_black = width_key_white/2;
    position = 0;

    // then we print the black keys
    for(int i = 0; i<N_NOTES_OCTAVE; i++)
    {
        if(layout[i] == 1)
        {
            //pRects[i] = scene_piano->addRect(QRectF(position * (width_key_white + 1) - width_key_black/2, 0, width_key_black, 150), QPen(Qt::NoPen), QBrush(Qt::black)); //QPen for border, QBrush for fill
            Keys[i] = new PianoKey(
                QRectF(position * (width_key_white + 1) - width_key_black / 2, 0, width_key_black, 150),
                PianoKey::Type::Black, i);  // i = note index
            scene_piano->addItem(Keys[i]);
        }
        else
        {
            position++;
        }
    }
}

int* Piano_ui::get_note_state()
{
    int* note_state = new int[N_NOTES_OCTAVE];
    for(int i = 0; i<N_NOTES_OCTAVE; i++)
    {
        note_state[i] = Keys[i]->get_state();
    }
    return note_state;
}