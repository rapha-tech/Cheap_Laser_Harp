#include "configFile.h"
#include <QDebug>


//     {
//        "soundfont_path": "C:\Users\Documents\sf2\Vintage.sf2",
//        "instrument_id": 12,
//        "volume_level": 80,
//        "midi_port_name": 0,
//        "notes_laser_0":[54]
//        "notes_laser_1":[56, 57]
//        "notes_laser_2":[59, 60]
//        "notes_laser_3":[54, 55, 56]
//        "notes_laser_4":[68]
//        "notes_laser_5":[54, 58, 56]
//     }

configFile::configFile(QString& configFilePath)
{
    // Default values if no file is found or if the field isn't present
    soundFont_path = QString();
    instrument_id = 0;
    volume = 80;
    midi_port_name = QString();
    m_accords = get_default_accord();

    yyjson_doc *doc = yyjson_read_file(configFilePath.toLocal8Bit(), 0, NULL, NULL);

    // Iterate over the root object
    if (doc)
    {
        yyjson_val *obj = yyjson_doc_get_root(doc);
        yyjson_obj_iter iter;
        yyjson_obj_iter_init(obj, &iter);
        yyjson_val *key, *val;
        while ((key = yyjson_obj_iter_next(&iter)))
        {
            if(!strcmp(yyjson_get_str(key), "soundfont_path"))
            {
                val = yyjson_obj_iter_get_val(key);
                soundFont_path = QString(yyjson_get_str(val));
            }
            else if(!strcmp(yyjson_get_str(key), "instrument_id"))
            {
                val = yyjson_obj_iter_get_val(key);
                instrument_id = yyjson_get_uint(val);
            }
            else if(!strcmp(yyjson_get_str(key), "volume_level"))
            {
                val = yyjson_obj_iter_get_val(key);
                volume = yyjson_get_uint(val);
            }
            else if(!strcmp(yyjson_get_str(key), "midi_port_name"))
            {
                val = yyjson_obj_iter_get_val(key);
                midi_port_name = QString(yyjson_get_str(val));
            }
            else if(!strcmp(yyjson_get_str(key), "notes_laser_0"))
            {
                setAccordJson(key, m_accords, 0);
            }
            else if(!strcmp(yyjson_get_str(key), "notes_laser_1"))
            {
                setAccordJson(key, m_accords, 1);
            }
            else if(!strcmp(yyjson_get_str(key), "notes_laser_2"))
            {
                setAccordJson(key, m_accords, 2);
            }
            else if(!strcmp(yyjson_get_str(key), "notes_laser_3"))
            {
                setAccordJson(key, m_accords, 3);
            }
            else if(!strcmp(yyjson_get_str(key), "notes_laser_4"))
            {
                setAccordJson(key, m_accords, 4);
            }
            else if(!strcmp(yyjson_get_str(key), "notes_laser_5"))
            {
                setAccordJson(key, m_accords, 5);
            }
        }

        qDebug() << "configFile : loaded config";
    }
    else
    {
        qDebug() << "configFile : read error";
    }

    // Free the doc
    yyjson_doc_free(doc);
}

void configFile::setAccordJson(yyjson_val* key, accord_t* accords, int id_laser)
{
    yyjson_val *val;
    yyjson_val *arr = yyjson_obj_iter_get_val(key);
    yyjson_arr_iter iter = yyjson_arr_iter_with(arr);
    int i = 0;
    while ((val = yyjson_arr_iter_next(&iter)))
    {
        accords[id_laser].notes[i++] = yyjson_get_uint(val);
    }
    accords[id_laser].n_notes = i;
}

void configFile::write(QString& conFigFilePath)
{
    // Create a mutable document. ie can be modified
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);

    // Create an object, the value's memory is held by doc.
    yyjson_mut_val *root = yyjson_mut_obj(doc);

    // SoundFont path
    yyjson_mut_val *val_soundFont_path = yyjson_mut_str(doc, soundFont_path.toLocal8Bit());
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "soundfont_path"), val_soundFont_path);

    // Instrument id
    yyjson_mut_val *num_0 = yyjson_mut_int(doc, instrument_id);
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "instrument_id"), num_0);

    // Volume level
    yyjson_mut_val *num_1 = yyjson_mut_int(doc, volume);
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "volume_level"), num_1);

    // Midi port id
    yyjson_mut_val *val_midi_port_name = yyjson_mut_str(doc, midi_port_name.toLocal8Bit());
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "midi_port_name"), val_midi_port_name);

    // notes arrays
    yyjson_mut_val *notes_arr_0 = yyjson_mut_arr(doc);
    yyjson_mut_val *notes_arr_1 = yyjson_mut_arr(doc);
    yyjson_mut_val *notes_arr_2 = yyjson_mut_arr(doc);
    yyjson_mut_val *notes_arr_3 = yyjson_mut_arr(doc);
    yyjson_mut_val *notes_arr_4 = yyjson_mut_arr(doc);
    yyjson_mut_val *notes_arr_5 = yyjson_mut_arr(doc);

    addNotesArray(doc, notes_arr_0, 0);
    addNotesArray(doc, notes_arr_1, 1);
    addNotesArray(doc, notes_arr_2, 2);
    addNotesArray(doc, notes_arr_3, 3);
    addNotesArray(doc, notes_arr_4, 4);
    addNotesArray(doc, notes_arr_5, 5);

    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "notes_laser_0"), notes_arr_0);
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "notes_laser_1"), notes_arr_1);
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "notes_laser_2"), notes_arr_2);
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "notes_laser_3"), notes_arr_3);
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "notes_laser_4"), notes_arr_4);
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "notes_laser_5"), notes_arr_5);

    // Set the document's root value.
    yyjson_mut_doc_set_root(doc, root);


    bool suc = yyjson_mut_write_file(conFigFilePath.toLocal8Bit(), doc, YYJSON_WRITE_PRETTY_TWO_SPACES, NULL, NULL);
    if (suc)
        qDebug() << "configFile : successfully saved";

    // Free the memory of doc and all values which is created from this doc.
    yyjson_mut_doc_free(doc);
}

void configFile::addNotesArray(yyjson_mut_doc* doc, yyjson_mut_val* arr, int id_laser)
{
    yyjson_mut_val *value;
    for(int i = 0; i < m_accords[id_laser].n_notes; i++)
    {
        value = yyjson_mut_int(doc, m_accords[id_laser].notes[i]);
        yyjson_mut_arr_append(arr, value);
    }
}

QString configFile::get_soundFont_path()
{
    return soundFont_path;
}

int configFile::get_instr_id()
{
    return instrument_id;
}

int configFile::get_volume()
{
    return volume;
}

QString configFile::get_port_name()
{
    return midi_port_name;
}

accord_t* configFile::getAccords()
{
    qDebug() << "configFile : getAccords";
    accord_t* accordcpy = new accord_t[6];
    for(int i = 0; i < 6; i++)
    {
        accordcpy[i].n_notes = m_accords[i].n_notes;
        for(int j = 0; j < m_accords[i].n_notes; j++)
        {
            accordcpy[i].notes[j] = m_accords[i].notes[j];
        }
    }
    return accordcpy;
}

accord_t* configFile::get_default_accord()
{
    qDebug() << "configFile : get_default_accord";
    int default_notes[] = {60, 62, 64, 65, 67, 69};
    accord_t* accords = new accord_t[6];

    for(int i = 0; i < 6; i++)
    {
        accords[i].n_notes = 1;
        accords[i].notes[0] = default_notes[i];
    }
    return accords;
}

void configFile::set_soundFont_path(QString& path)
{
    soundFont_path = path;
}

void configFile::set_instr_id(int id)
{
    instrument_id = id;
}

void configFile::set_volume(int vol)
{
    volume = vol;
}

void configFile::set_port_name(QString& name)
{
    midi_port_name = name;
}

void configFile::set_accords(accord_t* accordcpy)
{
    qDebug() << "configFile : set_accords";
    delete m_accords;
    m_accords = new accord_t[6];
    for(int i = 0; i < 6; i++)
    {
        m_accords[i].n_notes = accordcpy[i].n_notes;
        for(int j = 0; j < accordcpy[i].n_notes; j++)
        {
            m_accords[i].notes[j] = accordcpy[i].notes[j];
        }
    }
}
