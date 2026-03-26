/*******************************************************************************
 *
 *      devdrvd_communication.c
 *
 ******************************************************************************/

//#define     USE_VERBOSE

#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>
#include    <malloc.h>

#include    "devdrvd_communication.h"

const char* package_pattern = "%c%c";
const char* weight_package_pattern = "%cW ;%u.%06u;%u;%f;%f;%u%c";
const char* discrete_package_pattern = "%cDI;%u.%06u;%u;%u%c";
const char* error_package_pattern = "%cER;%u;%u;%s%c";

#define     PTERM_NOPACKET          0
#define     PTERM_PACKET            1
#define     RECBUF_SIZE             512

static  char    * pckt_buf          = NULL;
static  char    rec_buf[RECBUF_SIZE];
static  int     pckt_cur_pos        = 0;
static  int     pckt_cur_len        = 0;
//static  int     buf_empty           = 1; //MJK
static  int     term                = PTERM_NOPACKET;

/*******************************************************************************
 *
 *      Global Variables
 *
 ******************************************************************************/

int             avrg_pckts          = 0;
int             good_pckts          = 0;
int             bad_pckts           = 0;

pckt_item_p parse_buffer(void * buf, size_t size)
{
    int                 i;
    //size_t              b_size;
    pckt_item_p         ret       = NULL;
    pckt_item_p         cptr      = NULL;
    pckt_item_p         ptr		  = NULL; // init MJK

    pckt_buf            = (char *)buf;

#ifdef      USE_VERBOSE
    printf("PARSE_BUFFER[%05d]: Called [CUR_POS = %4d; CUR_LEN = %4d; TERM = %d]!\n",
        __LINE__, pckt_cur_pos, pckt_cur_len, term);
#endif

    for (i = 0; i < (int)size; i++)
    {
        rec_buf[pckt_cur_pos]     = *(pckt_buf + i);
        pckt_cur_len++;

        switch (rec_buf[pckt_cur_pos])
        {
            case    START_CHARACTER:
            {
#ifdef      USE_VERBOSE
    printf("PARSE_BUFFER[%05d]: Called <START_CHAR> [CUR_POS = %4d; CUR_LEN = %4d; TERM = %d]!\n",
        __LINE__, pckt_cur_pos, pckt_cur_len, term);
#endif

                if (term != PTERM_NOPACKET)
                {
                    bad_pckts++;

                    rec_buf[pckt_cur_pos = 0]   = *(pckt_buf + i);
                    pckt_cur_len                = 0;
                }
                else
                {
                    term        = PTERM_PACKET;

                    avrg_pckts++;
                }

                pckt_cur_pos++;

                break;
            }

            case    STOP_CHARACTER:
            {

#ifdef      USE_VERBOSE
    printf("PARSE_BUFFER[%05d]: Called <STOP_CHAR> [CUR_POS = %4d; CUR_LEN = %4d; TERM = %d]!\n",
        __LINE__, pckt_cur_pos, pckt_cur_len, term);
#endif

                cptr                = (struct __pckt_item_t*)calloc(1, sizeof(struct __pckt_item_t));

                if (cptr != NULL)
                {
                    term            = PTERM_NOPACKET;
                    cptr->pckt_size = pckt_cur_len;
                    cptr->pckt_ptr  = malloc(pckt_cur_len);
                    cptr->next      = NULL;

                    memcpy(cptr->pckt_ptr, rec_buf, pckt_cur_len);

                    if (ret == 0)
                    {
                        ret     = ptr      = cptr;
                    }
                    else
                    {
                        ptr->next           = cptr;
                        ptr                 = cptr;
                    }


                    good_pckts++;

                    pckt_cur_pos        = pckt_cur_len = 0;
                }
                else
                {

                }

                break;
            }

            default:
            {
                if (term == PTERM_NOPACKET)
                {

                }
                else
                {
                    pckt_cur_pos++;
                }
            }
        }

//        pckt_cur_pos++;
    }

#ifdef      USE_VERBOSE
    printf("PARSE_BUFFER[%05d]: return = %p!\n",
        __LINE__, ret);
#endif

    return  ret;
}

size_t pack_weight(char** data, const struct weight_struct * weight)
{
  /**
   * @brief buffer
   * @note verify the needed buffer length
   */
  char buffer [256];

  *data = 0;

  //const char* weight_package_pattern = "%cW ;%u.%u;%u;%g;%g;%u%c";
  size_t len = sprintf(buffer,  weight_package_pattern,
                    START_CHARACTER,                    
                    weight->ts.tv_sec,
                    weight->ts.tv_usec,
                    weight->channel,
                    weight->weight,
                    weight->tare,
                    weight->status,
                    STOP_CHARACTER);

  if(len == 0) {
    printf("sprintf failed!\n");
    return len;
   }

  // len++; // for end character

  *data = (char*)malloc(len*sizeof(char));
  strncpy(*data, buffer, len);

  return len;
}

void unpack_weight(struct weight_struct* weight, const char *data)
{
  // "%cW ;%u;%u.%u;%g;%g;%u"
  // first char should be STX?
  char* start = 0;
  start = strstr(data, ";");

  weight->ts.tv_sec = strtoul (++start, &start, 10);
  weight->ts.tv_usec = strtoul (++start, &start, 10);

  weight->channel = strtoul (++start, &start, 10);

  weight->weight = strtod(++start, &start);
  weight->tare = strtod(++start, &start);

  weight->status = strtoul(++start, &start, 10);
}

size_t pack_discrete(char ** data, const struct discrete_struct * dicrete)
{
  /**
   * @brief buffer
   * @note verify the needed buffer length
   */
  char buffer [256];

  *data = 0;
// const char* discrete_package_pattern = "%cDI;%u.%u;%u;%u%c";
  size_t len = sprintf(buffer,  discrete_package_pattern,
                    START_CHARACTER,                    
                    dicrete->ts.tv_sec,
                    dicrete->ts.tv_usec,
                    dicrete->channel,
                    dicrete->value,
                    STOP_CHARACTER);

  if(len == 0) {
    printf("sprintf failed!\n");
    return len;
   }

  //len++; // for end character

  *data = (char*)malloc(len*sizeof(char));
  strncpy(*data, buffer, len);

  return len;
}

void unpack_discrete(struct discrete_struct * dicrete, const char *data)
{
  char* start = 0;
  start = strstr(data, ";");

  dicrete->ts.tv_sec = strtoul (++start, &start, 10);
  dicrete->ts.tv_usec = strtoul (++start, &start, 10);
  dicrete->channel= strtoul (++start, &start, 10);
  dicrete->value = strtoul (++start, &start, 10);
}

size_t pack_error(char ** data, const struct error_struct * error)
{
    /**
     * @brief buffer
     * @note verify the needed buffer length
     */
    char buffer [256];

    *data = 0;

    // const char* discrete_package_pattern = "%cDI;%u;%u;%u";
    size_t len = sprintf(buffer,  error_package_pattern,
                      START_CHARACTER,
                      error->timestamp,
                      error->code,
                      error->message);

    if(len == 0) {
      printf("sprintf failed!\n");
      return len;
     }

    len++; // for end character

    *data = (char*)malloc(len*sizeof(char));
    strncpy(*data, buffer, len);

    return len;
}

void unpack_error(struct error_struct * error, const char *data)
{
    char* start = 0;
    start = strstr(data, ";");

    error->timestamp = strtoul (++start, &start, 10);
    error->code = strtoul (++start, &start, 10);
}
