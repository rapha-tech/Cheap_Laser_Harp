#include "configfile.h"

//     {
//        "soundfont_path": "C:\Users\Documents\sf2\Vintage.sf2",
//        "instrument_id": 12,
//        "volume_level": 80,
//        "Midi_port_id": 0,
//        "notes_laser_0":[54]
//        "notes_laser_1":[56, 57]
//        "notes_laser_2":[59, 60]
//        "notes_laser_3":[54, 55, 56]
//        "notes_laser_4":[68]
//        "notes_laser_5":[54, 58, 56]
//     }

configFile::configFile(QString& conFigFilePath)
{
    // Default values if no file is found
    SoundFont_path = QString();
    instrument_id = 0;
    volume = 80;
    port_id = 0;

    // Read the JSON file
    yyjson_doc *doc = yyjson_read_file(conFigFilePath.toLocal8Bit().constData(), 0, NULL, NULL);

    // Iterate over the root object
    if (doc) {
        yyjson_val *obj = yyjson_doc_get_root(doc);
        yyjson_obj_iter iter;
        yyjson_obj_iter_init(obj, &iter);
        yyjson_val *key, *val;
        while ((key = yyjson_obj_iter_next(&iter))) {
            if(!strcmp(yyjson_get_str(key), "soundfont_path"))
            {
                val = yyjson_obj_iter_get_val(key);
                SoundFont_path = QString(yyjson_get_str(val));
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
            else if(!strcmp(yyjson_get_str(key), "Midi_port_id"))
            {
                val = yyjson_obj_iter_get_val(key);
                port_id = yyjson_get_uint(val);
            }
        }
    } else {
        printf("read error");
    }

    // Free the doc
    yyjson_doc_free(doc);
}


void configFile::write(QString& conFigFilePath)
{
    // Create a mutable document. ie can be modified
    yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);

    // Create an object, the value's memory is held by doc.
    yyjson_mut_val *root = yyjson_mut_obj(doc);

    // SoundFont path
    yyjson_mut_val *val_SoundFont_path = yyjson_mut_str(doc, SoundFont_path.toLocal8Bit().constData());
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "soundfont_path"), val_SoundFont_path);

    // Instrument id
    yyjson_mut_val *num_0 = yyjson_mut_int(doc, instrument_id);
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "instrument_id"), num_0);

    // Volume level
    yyjson_mut_val *num_1 = yyjson_mut_int(doc, volume);
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "volume_level"), num_1);

    // Midi port id
    yyjson_mut_val *num_2 = yyjson_mut_int(doc, port_id);
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "Midi_port_id"), num_2);

    // notes arrays
    yyjson_mut_val *notes_arr_0 = yyjson_mut_arr(doc);
    yyjson_mut_val *value00 = yyjson_mut_int(doc, 53);
    yyjson_mut_val *value01 = yyjson_mut_int(doc, 55);
    yyjson_mut_arr_append(notes_arr_0, value00);
    yyjson_mut_arr_append(notes_arr_0, value01);
    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "notes_laser_0"), notes_arr_0);

    // Set the document's root value.
    yyjson_mut_doc_set_root(doc, root);


    bool suc = yyjson_mut_write_file(conFigFilePath.toLocal8Bit().constData(), doc, YYJSON_WRITE_PRETTY_TWO_SPACES, NULL, NULL);
    if (suc) printf("OK");

    // Free the memory of doc and all values which is created from this doc.
    yyjson_mut_doc_free(doc);
}

QString configFile::get_soundFont_path()
{
    return SoundFont_path;
}

int configFile::get_instr_id()
{
    return instrument_id;
}

int configFile::get_volume()
{
    return volume;
}

int configFile::get_port_id()
{
    return port_id;
}


void configFile::set_soundFont_path(QString& path)
{
    SoundFont_path = path;
}

void configFile::set_instr_id(int id)
{
    instrument_id = id;
}

void configFile::set_volume(int vol)
{
    volume = vol;
}

void configFile::set_port_id(int id)
{
    port_id = id;
}
