#include <stdio.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

extern SystemConfiguration_t Configuration;

extern int getIdxChannel( int channel);

/*
 * int processingCurMeasurement( VDB *pVDB)
 */
int processingCurMeasurement( VDB *pVDB)
{
	fprintf( stdout, "processingCurMeasurement: %p\n", (void*)pVDB);

	if( pVDB->fun) {
		return (*pVDB->fun)( pVDB); 
	}

	if( !(Configuration.flag_system & FL_SYSTEM_CONFIGURATION_READY))
		return MATRIXLIB_SYSTEM_NOT_CONFIGURATION;
	
	int iRc = MATRIXLIB_OK;
	
	CurMeasurement_t *curMeasurement = (CurMeasurement_t *)pVDB->pParam;
	int idx = getIdxChannel( curMeasurement->channel);
	if( idx == -1) {
		return MATRIXLIB_PARAM_ERROR;
	}
	
	Channel_t* pChannel = &Configuration.channelConfiguration[idx];
// канал работает в режиме RAW_DATA	?
	if( pChannel->flag_channel & FL_CHANNEL_RAW_DATA) {
// нет линейных участков, не можем преобразовать		
		if( (int)pChannel->linearSections.size() < 2) {
			curMeasurement->weight_value = 0;
			return MATRIXLIB_LINEARITAZION_ERROR; 
		}
		int i;
		for( i = (int)pChannel->linearSections.size()-1; i>=0; i--) {
			if( curMeasurement->raw_value > pChannel->linearSections[i].raw_value) {
				curMeasurement->weight_value = curMeasurement->raw_value * pChannel->linearSections[i].ratio;
				break;
			}
		}
		// получили отчсет ниже точки Нуля
		if( i < 0) {
			curMeasurement->weight_value = curMeasurement->raw_value * pChannel->linearSections[0].ratio;
		}

// делаем усреденение для калибровки		
		if( Configuration.flag_system & FL_SYSTEM_SERVICE_MODE) {
// каждое значение			
			if( pChannel->averag_value.averag_time == 0) {
				pChannel->flag_channel |= FL_CHANNEL_AVERAG_VALUE;
				pChannel->averag_value.averag_weight_value = curMeasurement->weight_value;
				pChannel->averag_value.averag_raw_value = curMeasurement->raw_value;
			}
			else {
				pChannel->averag_value.averag_weight_value += curMeasurement->weight_value;
				pChannel->averag_value.averag_weight_value /= 2;
				pChannel->averag_value.averag_raw_value += curMeasurement->raw_value;
				pChannel->averag_value.averag_raw_value /= 2;
				
// если время усреднения прошло, выставим флаг 				
				if( difftime( time(NULL), pChannel->averag_value.start_time) >= pChannel->averag_value.averag_time)
					pChannel->flag_channel |= FL_CHANNEL_AVERAG_VALUE;
			}
			
		}
	}
	else {
		curMeasurement->weight_value = curMeasurement->raw_weight_value;
	}
	
	return iRc;
}
