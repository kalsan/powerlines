/**
   Copyright 2009-2015 Sandro Kalbermatter

    LICENSE:
    This program is licenced under GPL v3.0, see http://www.gnu.org/licenses/gpl-3.0
    It comes with absolutely no warranty.
**/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <list>
#include <algorithm>

typedef struct Setting{
    std::string id;      ///< Identifier of the setting, used by client
    std::string descr;   ///< Description for storage in the settings file
    int value;           ///< Value which this setting is set to. Note that we can store an integer only.
    int defaultValue;    ///< On init / reset / failed load, value will be set to this.
    bool setByFile;      ///< This is true means that the value of this setting is the same as in the file.
} Setting;

class Settings
{
public:  // For description of the functions, see settings.cpp
    Settings(std::string newFileName);
    ~Settings();
    void addSetting(std::string id, std::string descr, int defaultValue);
    void set(std::string id, int value);
    int get(std::string id);
    void reset(std::string id);
    void resetAll();
    int load(bool fix);
    int load();
    int loadNoFix();
    int save();

private:
    Setting* find(std::string id);

    std::list<Setting*> settings;  ///< This list contains the actual settings
    std::string fileName;          ///< Name of the file we store the settings into / of which we read the settings from
};

#endif // SETTINGS_H
