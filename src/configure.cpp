#include "configure.h"

CConfigure::CConfigure()
{
}

CConfigure::~CConfigure()
{
}


int CConfigure::parse_array(const char* value, std::vector<std::string>& array)
{
        int len = strlen(value);

        if(value[0] != '{' && value[len - 1] != '}') {
                TRACE(2, "value format error. value: " << value);
                return -1;
        }

        char buffer[1024];
        memset(buffer, 0, 1024);
        char* start = (char*)(value + 1);
        char* end = (char*)(value + 1);

        while(end != '\0') {
                if(*end == ',' || *end == '}') {
                        int a = end - start;
                        memcpy(buffer, start, a);
                        array.push_back(buffer);
                        memset(buffer, 0, 1024);
                        start = end + 1;
                }

                if(*end == '}') {
                        break;
                }

                end++;
        }

        return 0;
}

void CConfigure::set_conf_file_name(const char* apFileName)
{
    _file_name = apFileName;
}

char* CConfigure::strstrip(char* s)
{
    if(strlen(s) > ASCIILINESZ) {
        return NULL;
    }
    
    static char l[ASCIILINESZ + 1];
    char* last ;
    
    if(s == NULL) return NULL ;
    
    while(isspace((int)*s) && *s) s++;
    
    memset(l, 0, ASCIILINESZ + 1);
    strcpy(l, s);
    last = l + strlen(l);
    
    while(last > l) {
        if(!isspace((int) * (last - 1)))
            break ;
            
        last -- ;
    }
    
    *last = (char)0;
    return (char*)l ;
}

line_status CConfigure::iniparser_line(
    char* input_line,
    char* section,
    char* key,
    char* value)
{
    line_status sta ;
    char        line[ASCIILINESZ + 1];
    int         len ;
    strcpy(line, strstrip(input_line));
    len = (int)strlen(line);
    sta = LINE_UNPROCESSED ;
    
    if(len < 1) {
        /* Empty line */
        sta = LINE_EMPTY ;
    } else if(line[0] == '#') {
        /* Comment line */
        sta = LINE_COMMENT ;
    } else if(line[0] == '[' && line[len - 1] == ']') {
        /* Section name */
        sscanf(line, "[%[^]]", section);
        strcpy(section, strstrip(section));
        strcpy(section, (section));
        sta = LINE_SECTION ;
    } else if(sscanf(line, "%[^=] = \"%[^\"]\"", key, value) == 2
              ||  sscanf(line, "%[^=] = '%[^\']'",   key, value) == 2
              ||  sscanf(line, "%[^=] = %[^;#]",     key, value) == 2) {
        /* Usual key=value, with or without comments */
        strcpy(key, strstrip(key));
        strcpy(key, (key));
        strcpy(value, strstrip(value));
        
        /*
        * sscanf cannot handle '' or "" as empty values
        * this is done here
        */
        if(!strcmp(value, "\"\"") || (!strcmp(value, "''"))) {
            value[0] = 0 ;
        }
        
        sta = LINE_VALUE ;
    } else if(sscanf(line, "%[^=] = %[;#]", key, value) == 2
              ||  sscanf(line, "%[^=] %[=]", key, value) == 2) {
        /*
        * Special cases:
        * key=
        * key=;
        * key=#
        */
        strcpy(key, strstrip(key));
        strcpy(key, (key));
        value[0] = 0 ;
        sta = LINE_VALUE ;
    } else {
        /* Generate syntax error */
        sta = LINE_ERROR ;
    }
    
    return sta ;
}

bool CConfigure::load()
{
    const char* ininame = _file_name.c_str();
    FILE* in = NULL;
    char line    [ASCIILINESZ + 1] ;
    char section [ASCIILINESZ + 1] ;
    char key     [ASCIILINESZ + 1] ;
    char val     [ASCIILINESZ + 1] ;
    int  last = 0 ;
    int  len = 0;
    int  lineno = 0 ;
    
    if((in = fopen(ininame, "r")) == NULL) {
        TRACE(1, "read configure file faild. file name: " << ininame);
        return false ;
    }
    
    memset(line,    0, ASCIILINESZ);
    memset(section, 0, ASCIILINESZ);
    memset(key,     0, ASCIILINESZ);
    memset(val,     0, ASCIILINESZ);
    last = 0 ;
    
    while(fgets(line + last, ASCIILINESZ - last, in) != NULL) {
        lineno++ ;
        len = (int)strlen(line) - 1;
        
        /* Safety check against buffer overflows */
        if(line[len] != '\n') {
            TRACE(1, "parse configure file faild. line number: " << lineno);
            fclose(in);
            return false ;
        }
        
        /* Get rid of \n and spaces at end of line */
        while((len >= 0) &&
                ((line[len] == '\n') || (isspace(line[len])))) {
            line[len] = 0 ;
            len-- ;
        }
        
        /* Detect multi-line */
        if(line[len] == '\\') {
            /* Multi-line value */
            last = len ;
            continue ;
        } else {
            last = 0 ;
        }
        
        switch(iniparser_line(line, section, key, val)) {
        case LINE_EMPTY:
        case LINE_SECTION:
        case LINE_COMMENT: {
                break ;
            }
            
        case LINE_VALUE: {
                if(!parse_value(key, val)) {
                    TRACE(1, "CConfigure::load 分析配置文件内容失败。行数：" << lineno);
                }
                
                break;
            }
            
        case LINE_ERROR: {
                TRACE(1, "CConfigure::load 分析配置文件失败。行数： " << lineno);
                break;
            }
            
        default: {
                TRACE(1, "default value, lineno:  " << lineno);
                break ;
            }
        }
        
        memset(line, 0, ASCIILINESZ);
        last = 0;
    }
    
    fclose(in);
    return true;
}


std::string CConfigure::GetStrParam(const std::string& astrKey, const std::string& astrSection, const std::string& astrCfgFile)
{
    const char* ininame = astrCfgFile.c_str();
    FILE* in;
    char line    [ASCIILINESZ + 1] ;
    char section [ASCIILINESZ + 1] ;
    char key     [ASCIILINESZ + 1] ;
    char val     [ASCIILINESZ + 1] ;
    int  last = 0;
    int  len;
    int  lineno = 0;
    
    if((in = fopen(ininame, "r")) == NULL) {
        TRACE(1, "CConfigure::load 打开配置文件失败。名字： " << ininame);
        return false ;
    }
    
    memset(line,    0, ASCIILINESZ);
    memset(section, 0, ASCIILINESZ);
    memset(key,     0, ASCIILINESZ);
    memset(val,     0, ASCIILINESZ);
    last = 0 ;
    bool lbFound = false;
    std::string lstrValue = "";
    
    while(!lbFound && fgets(line + last, ASCIILINESZ - last, in) != NULL) {
        lineno++ ;
        len = (int)strlen(line) - 1;
        
        /* Safety check against buffer overflows */
        if(line[len] != '\n') {
            TRACE(1, "CConfigure::load 加载配置文件失败。行数： " << lineno);
            fclose(in);
            return false ;
        }
        
        /* Get rid of \n and spaces at end of line */
        while((len >= 0) &&
                ((line[len] == '\n') || (isspace(line[len])))) {
            line[len] = 0 ;
            len-- ;
        }
        
        /* Detect multi-line */
        if(line[len] == '\\') {
            /* Multi-line value */
            last = len ;
            continue ;
        } else {
            last = 0 ;
        }
        
        switch(iniparser_line(line, section, key, val)) {
        case LINE_EMPTY:
        case LINE_COMMENT: {
                break ;
            }
            
        case LINE_SECTION:
            break ;
            
        case LINE_VALUE: {
                //parse_value(key, val);
                if(astrKey == key && astrSection == section) {
                    lbFound = true;
                    lstrValue = val;
                }
                
                break;
            }
            
        case LINE_ERROR: {
                TRACE(1, "CConfigure::load 分析配置文件失败。行数： " << lineno);
                break;
            }
            
        default: {
                break ;
            }
        }
        
        memset(line, 0, ASCIILINESZ);
        last = 0;
    }
    
    fclose(in);
    return lstrValue;
}
int CConfigure::GetIntParam(const std::string& astrKey, const std::string& astrSection, const std::string& astrCfgFile)
{
    std::string lstrValue = GetStrParam(astrKey, astrSection, astrCfgFile);
    return atoi(lstrValue.c_str());
}

