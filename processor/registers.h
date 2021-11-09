enum registers {
    #define DEF_REG(enum_name, name) \
        enum_name,
    
    #include "registers_defenitions.h"

    #undef DEF_REG
};
