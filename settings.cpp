#include "settings.h"

Settings::Settings(std::string newFileName){
    fileName=newFileName;
}

Settings::~Settings(){
    while(!settings.empty()){
        delete settings.front();
        settings.pop_front();
    }
}

void Settings::addSetting(std::string id, std::string descr, int defaultValue){
    Setting* newSetting=new Setting;
    newSetting->id=id;
    newSetting->descr=descr;
    newSetting->value=defaultValue;
    newSetting->defaultValue=defaultValue;
    newSetting->setByFile=false;
    settings.push_back(newSetting);
}

int Settings::set(std::string id, int value){
    find(id)->value=value;
    return 0;
}

int Settings::get(std::string id){
    return find(id)->value;
}

void Settings::reset(std::string id){
    Setting* item=find(id);
    item->value=item->defaultValue;
    item->setByFile=false;
}

void Settings::resetAll(){
    for(std::list<Setting*>::iterator it=settings.begin(); it != settings.end(); ++it){
        reset((*it)->id);
    }
}

int Settings::load(bool fix){
    bool somethingIsMissing=false;
    FILE* fSettings=fopen(fileName.c_str(),"r");
    if(!fSettings){
        fprintf(stderr,"WARNING: Settings file \"%s\" not found! Loading default values.\n",fileName.c_str());
        resetAll();
        save();
        return 1;
    }
    char lineBuf[500],id[500];
    int val;
    while(fgets(lineBuf,500,fSettings)){
        if(sscanf(lineBuf,"%s = %d\t\t%*s",id,&val) != 2){
            fprintf(stderr,"WARNING: Settings file \"%s\" contains an invalid line! You should correct or remove this line: %s\n",fileName.c_str(),lineBuf);
            fprintf(stderr,"Read %d arguments. id is %s and val is %d\n",sscanf(lineBuf,"%s = %d\t\t%*s",id,&val),id,val);
        }else{
            set(id,val);
            find(id)->setByFile=true;
        }
    }
    for(std::list<Setting*>::iterator it=settings.begin(); it != settings.end(); ++it){
        if(!(*it)->setByFile){
            fprintf(stderr,"WARNING: Setting \"%s\" not found in settings file! This value will be set to default value %d\n",(*it)->id.c_str(),(*it)->value);
            reset((*it)->id);
            somethingIsMissing=true;
        }
    }
    fclose(fSettings);

    if(fix && somethingIsMissing){
        fSettings=fopen(fileName.c_str(),"a");
        if(!fSettings){
            fprintf(stderr,"WARNING: Cannot open \"%s\" for writing! File will not be fixed.\n",fileName.c_str());
        }else{
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
    return load(true);
}

int Settings::loadNoFix(){
    return load(false);
}

int Settings::save(){
    FILE* fSettings=fopen(fileName.c_str(),"w");
    if(!fSettings){
        fprintf(stderr,"WARNING: Cannot open \"%s\" for writing! Your settings will NOT be saved.\n",fileName.c_str());
        return 1;
    }

    for(std::list<Setting*>::iterator it=settings.begin(); it != settings.end(); ++it){
        fprintf(fSettings,"%s = %d\t%s\n",(*it)->id.c_str(), (*it)->value, (*it)->descr.c_str());
    }
    fclose(fSettings);
    return 0;
}

Setting* Settings::find(std::string id){
    std::list<Setting*>::iterator it=settings.begin();
    for(; it != settings.end() && (**it).id!=id; ++it);
    if(it==settings.end()){return 0;}
    return *it;
}
