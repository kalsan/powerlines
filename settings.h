#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <list>
#include <algorithm>

typedef struct Setting{
    std::string id;
    std::string descr;
    int value;
    int defaultValue;
    bool setByFile;
} Setting;

class Settings
{
public:
    Settings(std::string newFileName);
    ~Settings();
    void addSetting(std::string id, std::string descr, int defaultValue);
    int set(std::string id, int value);
    int get(std::string id);
    void reset(std::string id);
    void resetAll();
    int load(bool fix);
    int load();
    int loadNoFix();
    int fix();
    int save();

private:
    Setting* find(std::string id);

    std::list<Setting*> settings;
    std::string fileName;
};

#endif // SETTINGS_H
