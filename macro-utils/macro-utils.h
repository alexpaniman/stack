#define MACRO_UTILS_NARG( ...) MACRO_UTILS_NARG_(__VA_ARGS__, MACRO_UTILS_RSEQ_N())

#define MACRO_UTILS_NARG_(...) MACRO_UTILS_128TH_ARG(__VA_ARGS__)

#define MACRO_UTILS_128TH_ARG(                                       \
         _001, _002, _003, _004, _005, _006, _007, _008, _009, _010, \
         _011, _012, _013, _014, _015, _016, _017, _018, _019, _020, \
         _021, _022, _023, _024, _025, _026, _027, _028, _029, _030, \
         _031, _032, _033, _034, _035, _036, _037, _038, _039, _040, \
         _041, _042, _043, _044, _045, _046, _047, _048, _049, _050, \
         _051, _052, _053, _054, _055, _056, _057, _058, _059, _060, \
         _061, _062, _063, _064, _065, _066, _067, _068, _069, _070, \
         _071, _072, _073, _074, _075, _076, _077, _078, _079, _080, \
         _081, _082, _083, _084, _085, _086, _087, _088, _089, _090, \
         _091, _092, _093, _094, _095, _096, _097, _098, _099, _100, \
         _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, \
         _111, _112, _113, _114, _115, _116, _117, _118, _119, _120, \
         _121, _122, _123, _124, _125, _126, _127, N, ...) N

#define MACRO_UTILS_RSEQ_N()                                         \
                      127,  126,  125,  124,  123,  122,  121,  120, \
          119,  118,  117,  116,  115,  114,  113,  112,  111,  110, \
          109,  108,  107,  106,  105,  104,  103,  102,  101,  100, \
           99,   98,   97,   96,   95,   94,   93,   92,   91,   90, \
           89,   88,   87,   86,   85,   84,   83,   82,   81,   80, \
           79,   78,   77,   76,   75,   74,   73,   72,   71,   70, \
           69,   68,   67,   66,   65,   64,   63,   62,   61,   60, \
           59,   58,   57,   56,   55,   54,   53,   52,   51,   50, \
           49,   48,   47,   46,   45,   44,   43,   42,   41,   40, \
           39,   38,   37,   36,   35,   34,   33,   32,   31,   30, \
           29,   28,   27,   26,   25,   24,   23,   22,   21,   20, \
           19,   18,   17,   16,   15,   14,   13,   12,   11,   10, \
            9,    8,    7,    6,    5,    4,    3,    2,    1,    0
