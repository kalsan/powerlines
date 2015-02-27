/**
  Copyright 2015 Sandro Kalbermatter
  License: GPL v3

  This class provides a simple interface for dealing with settings.

  First, a Settings object must be created, specifying a file name. If you create multiple such
    objects, you're dealing with completely independent settings sets, using different files.
  Second, all settings to be taken care of must be made known by calling addSetting.
    You must provide: - an identifier string, e.g. "genFontSize",
                      - a description that will be written to the settings file,
                        e.g. "Set Generel Font Size here."
                      - a default integer value used for initializing the setting, when
                        resetting it, or when we fail loading it from the file.
                        e.g. 12
  To deal with the value of a setting, use set(), reset() and get(), specifying the setting's identifier.
  To reset all values to their default, use resetAll().
  To read / write all the settings handled by your Setting object from / to disk, use load() / save().
    If you want to avoid that missing lines are automatically appended to the file, use loadNoFix().

  Have fun!

  **/

#include "settings.h"

Settings::Settings(std::string newFileName){
    /// Constructor

    fileName=newFileName;
}

Settings::~Settings(){
    /// Destructor

    while(!settings.empty()){ // Delete all settings of this Settings object.
        delete settings.front();
        settings.pop_front();
    }
}

void Settings::addSetting(std::string id, std::string descr, int defaultValue){
    /// This makes a setting being known to the program. It can then be used.
    /// You may NOT access a setting that hasn't been added before by this function!

    Setting* newSetting=new Setting;
    newSetting->id=id;
    newSetting->descr=descr;
    newSetting->value=defaultValue;
    newSetting->defaultValue=defaultValue;
    newSetting->setByFile=false;
    settings.push_back(newSetting);
}

void Settings::set(std::string id, int value){
    /// Set value of a single setting.
    /// The setting must have been added by addSetting before.

    Setting* target=find(id);
    if(!target){
        fprintf(stderr,"FATAL ERROR: Tried to set value of inexisting setting \"%s\"! This is a bug, it should never happen.\n",id.c_str());
        exit(EXIT_FAILURE);
    }
    target->value=value;
    target->setByFile=false;
}

int Settings::get(std::string id){
    /// Read value of a single setting.
    /// The setting must have been added by addSetting before.
    /// Returns value defined by this setting.

    Setting* target=find(id);
    if(!target){
        fprintf(stderr,"FATAL ERROR: Tried to read value of inexisting setting \"%s\"! This is a bug, it should never happen.\n",id.c_str());
        exit(EXIT_FAILURE);
    }
    return target->value;
}

void Settings::reset(std::string id){
    /// Reset a setting to its default value
    /// The setting must have been added by addSetting before.

    Setting* target=find(id);
    if(!target){
        fprintf(stderr,"FATAL ERROR: Tried to reset value of inexisting setting \"%s\"! This is a bug, it should never happen.\n",id.c_str());
        exit(EXIT_FAILURE);
    }
    target->value=target->defaultValue;
    target->setByFile=false;
}

void Settings::resetAll(){
    /// Reset all settings in this Settings object

    for(std::list<Setting*>::iterator it=settings.begin(); it != settings.end(); ++it){
        (*it)->value=(*it)->defaultValue; // This is faster than the use of reset(). The code shall be the same.
        (*it)->setByFile=false;
    }
}

int Settings::load(bool fix){
    /// Open the file at 'fileName' and load its known settings.
    /// If 'fix' is true, append any settings that were added by addSetting but which are not in the file.
    /// Note that fix will not remove anything from the settings file.
    /// Returns 0 on success.

    bool somethingIsMissing=false; // This becomes true if we know that a setting is in this Settings object,
                                   // but not in the file. Avoids a usless parse through the settings if everything is fine.
    FILE* fSettings=fopen(fileName.c_str(),"r");
    if(!fSettings){
        // If the settings file does not exist, create it and restore defaults, as the values should have been overwritten by the ones stored in the file.
        fprintf(stderr,"WARNING: Settings file \"%s\" not found! Loading default values.\n",fileName.c_str());
        resetAll();
        save();
        return 1;
    }
    // If the file exists, read from it and check each line for correctness. If so, load its value into the appropriate setting.
    char lineBuf[500],id[500];
    int val;
    while(fgets(lineBuf,500,fSettings)){
        if(sscanf(lineBuf,"%s = %d\t\t%*s",id,&val) != 2){
            // Syntax error in this line
            fprintf(stderr,"WARNING: Settings file \"%s\" contains an invalid line! You should correct or remove this line: %s\n",fileName.c_str(),lineBuf);
        }else{
            // Correct syntax: try to load from it
            Setting* target=find(id);
            if(target){
                // Successful load
                set(id,val);
                target->setByFile=true; // Register that this value comes freshly out of the file.
            }else{
                // Correct syntax, but the setting is unknown to the program at this point
                fprintf(stderr,"WARNING: Settings file \"%s\" contains an invalid setting identifier! You should correct or remove this line: %s\n",fileName.c_str(),lineBuf);
            }
        }
    }
    // We're done looking at the file, now figure out if all settings in this Setting object have been found in the file
    for(std::list<Setting*>::iterator it=settings.begin(); it != settings.end(); ++it){
        if(!(*it)->setByFile){
            // We know that this setting should be in the file, but it's not. Load default instead.
            fprintf(stderr,"WARNING: Setting \"%s\" not found in settings file! This setting will be set to default value %d\n",(*it)->id.c_str(),(*it)->value);
            reset((*it)->id);
            somethingIsMissing=true;
        }
    }
    fclose(fSettings);

    // If the settings file is incomplete and we're supposed to fix that, add missing settings
    if(fix && somethingIsMissing){
        fSettings=fopen(fileName.c_str(),"a");
        if(!fSettings){
            // Read-only location, we cannot write changes
            fprintf(stderr,"WARNING: Cannot open \"%s\" for writing! File will not be fixed.\n",fileName.c_str());
        }else{
            // Parse all known settings and add those which are missing in the file
            for(std::list<Setting*>::iterator it=settings.begin(); it != settings.end(); ++it){
                if(!(*it)->setByFile){
                    fprintf(stderr,"INFO about warning above: Adding Setting \"%s\" to file \"%s\".\n",(*it)->id.c_str(),fileName.c_str());
                    fprintf(fSettings,"%s = %d\t\t%s\n",(*it)->id.c_str(), (*it)->value, (*it)->descr.c_str());
                }
            }
        }
        fclose(fSettings);
    }
    return 0;
}

int Settings::load(){
    /// Open the file at 'fileName' and load its known settings.
    /// Append any settings that were added by addSetting but which are not in the file.
    /// Returns 0 on success.

    return load(true);
}

int Settings::loadNoFix(){
    /// Open the file at 'fileName' and load its known settings.
    /// Will read from the file but not write to it, unless the settings file dos not exist, in which case this function behaves like load()
    /// Returns 0 on success.

    return load(false);
}

int Settings::save(){
    /// Overwrite the file at 'fileName' and store all settings added by addSetting
    /// Returns 0 on success.

    // Try to open for writing
    FILE* fSettings=fopen(fileName.c_str(),"w");
    if(!fSettings){
        // Read-only location, we cannot write changes
        fprintf(stderr,"WARNING: Cannot open \"%s\" for writing! Your settings will NOT be saved.\n",fileName.c_str());
        return 1;
    }
    // File is open, now write down all settings
    for(std::list<Setting*>::iterator it=settings.begin(); it != settings.end(); ++it){
        fprintf(fSettings,"%s = %d\t%s\n",(*it)->id.c_str(), (*it)->value, (*it)->descr.c_str());
    }
    fclose(fSettings);
    return 0;
}

Setting* Settings::find(std::string id){
    /// Search an id in all settings added by addSetting
    /// If not found, returns NULL.
    /// This is a private function.

    // Parse through all known settings until hit or end of list
    std::list<Setting*>::iterator it=settings.begin();
    for(; it != settings.end() && (**it).id!=id; ++it);
    if(it==settings.end()){
        // Nothing found
        return NULL;
    }
    return *it;
}
