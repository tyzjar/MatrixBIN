#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

SystemConfiguration_t	Configuration;		

int (*VpiFunc[VPI_MAX_FUNCTION])(VDB *);

/*
 * initialization
 */
void initialization()
{
	memset( (void*)&Configuration, 0, sizeof( Configuration));

	memset( VpiFunc, 0, sizeof( VpiFunc));
	
	VpiFunc[VPI_LIB_VERSION] = libVersion;
	VpiFunc[VPI_GET_TAC] = getTAC;
	VpiFunc[VPI_LOAD_CONFIGURATION] = loadConfiguration;
	VpiFunc[VPI_SAVE_CONFIGURATION] = saveConfiguration;
	
	VpiFunc[VPI_SET_MAXIMUM] = setMaximum;
	VpiFunc[VPI_SET_MINIMUM] = setMinimum;
	VpiFunc[VPI_SET_RANGE_MODE] = setRangeMode;
	VpiFunc[VPI_SET_INCREMENT_SIZE] = setIncrementSize;
	VpiFunc[VPI_SET_DEC_POINT] = setDecPoint;
	VpiFunc[VPI_SET_CALIBRATE_ZERO] = setCalibrateZero;
	VpiFunc[VPI_SET_CALIBRATE_GANE] = setCalibrateGane;
	VpiFunc[VPI_SET_CALIBRATE_ZERO_CORRECTION] = setCalibrateZeroCorrection;
	VpiFunc[VPI_SET_LINEARIZATION] = setLinearization;
	VpiFunc[VPI_SAVE_CALIBRATION] = saveCalibration;
	VpiFunc[VPI_SET_WARM_UP_TIME] = setWarmUpTime;
	
	VpiFunc[VPI_ZERO_TRACKING] = zeroTracking;
	VpiFunc[VPI_SET_ZERO_MANUAL_RANGE] = setZeroManualRange;
	VpiFunc[VPI_SET_ZERO_INITIAL_RANGE] = setZeroInitialRange;
	
	VpiFunc[VPI_ZERO_SET] = zeroSet;
	VpiFunc[VPI_ZERO_RESET] = zeroReset;
	VpiFunc[VPI_ZERO_SAVE] = zeroSave;
	VpiFunc[VPI_ZERO_RESTORE] = zeroRestore;

	VpiFunc[VPI_SET_MAXIMUM_CHANNEL] = setMaximumChannel;
	VpiFunc[VPI_SET_DISCRET] = setDiscret;
	VpiFunc[VPI_SET_AVERAG_TIME] = setAveragTime;
	VpiFunc[VPI_SET_ZERO_INDICATION] = setZeroIndication;
		
	VpiFunc[VPI_TARE_MODE] = tareMode;
	VpiFunc[VPI_TARE_SET] = tareSet;
	VpiFunc[VPI_TARE_RESET] = tareReset;
	VpiFunc[VPI_TARE_PRESET] = tarePreset;
	VpiFunc[VPI_TARE_SAVE] = tareSave;
	VpiFunc[VPI_TARE_RESTORE] = tareRestore;
	VpiFunc[VPI_TARE_AUTOSET] = tareAutoSet;
	VpiFunc[VPI_TARE_AUTOCLEAR] = tareAutoClear;
	
	VpiFunc[VPI_PROCESSING_CUR_MEASUREMENT] = processingCurMeasurement;
	VpiFunc[VPI_FILTERING_PERFORM] = filteringPerform;
	VpiFunc[VPI_CALCULATE_WEIGHT] = calculateWeight;
	VpiFunc[VPI_NORMALIZE_WEIGHT] = normalizeWeight;
	VpiFunc[VPI_SWITCH_UNIT] = switchUnit;
	VpiFunc[VPI_IS_MOTION] = isMotion;
	VpiFunc[VPI_MODE_X10] = modeX10;
	VpiFunc[VPI_SET_CHANEL_ON_OFF] = setChannelOnOff;
	
	VpiFunc[VPI_PRINT] = print;
	VpiFunc[VPI_SAVE_RESULTS] = saveResults;
	
	VpiFunc[VPI_SET_WIM_MAX_CAPACITY] = setWimMaxCapacity;
	VpiFunc[VPI_SET_WIM_MIN_CAPACITY] = setWimMinCapacity;
	VpiFunc[VPI_SET_WIM_MAX_PLATFORM] = setWimMaxPlatform;
	VpiFunc[VPI_SET_WIM_MIN_PLATFORM] = setWimMinPlatform;
	VpiFunc[VPI_SET_WIM_MAX_WAGON_MASS] = setWimMaxWagonMass;
	VpiFunc[VPI_SET_WIM_MIN_WAGON_MASS] = setWimMinWagonMass;
	VpiFunc[VPI_SET_WIM_MAX_TRAIN_MASS] = setWimMaxTrainMass;
	VpiFunc[VPI_SET_WIM_MIN_TRAIN_MASS] = setWimMinTrainMass;
	VpiFunc[VPI_SET_WIM_MAX_SPEED] = setWimMaxSpeed;
	VpiFunc[VPI_SET_WIM_MIN_SPEED] = setWimMinSpeed;
	VpiFunc[VPI_SET_WIM_MAX_TRANSIT_SPEED] = setWimMaxTransitSpeed;
	
	VpiFunc[VPI_SET_WIM_ADJUSTMENT] = setWimAdjusment;
	VpiFunc[VPI_WIM_CALCULATE_SPEED] = wimCalculateSpeed;
	VpiFunc[VPI_WIM_SOLVER] = wimSolver;
	VpiFunc[VPI_WIM_PRINT] = wimPrint;
	VpiFunc[VPI_WIM_SAVE_RESULTS] = wimSaveResults;
	VpiFunc[VPI_RESET_TO_FACTORY] = resetToFactory;
}

/*********************************/
/* VPI main entry point          */
/*********************************/
int VpiMain(VDB *pVDB)
{
   int iRc,
   	   iFunc;

   iFunc = pVDB->function;

   /* Check if the function number received is legal */
   if ( iFunc > (int)(sizeof(VpiFunc) / sizeof(VpiFunc[0])) )
      return MATRIXLIB_RECOVERY_FUNCTION_ERROR;

   if ( VpiFunc[iFunc] == 0 )
      return MATRIXLIB_RECOVERY_FUNCTION_ERROR;

   iRc = ( *VpiFunc[iFunc] )(pVDB); /* call the required function */

   return iRc;

} //VpiMain()


int getIdxScale( int id_scale)
{
	int idx = -1;

	for( int i=0; i < (int)Configuration.scaleConfiguration.size(); i++) {
		if( Configuration.scaleConfiguration[i].id_scale == id_scale) {
			idx = i;
			break;
		}	
	}
	return idx;

}


int getIdxChannel( int channel)
{
	int idx = -1;
	
	for( int i=0; i < (int)Configuration.channelConfiguration.size(); i++) {
		if( Configuration.channelConfiguration[i].channel == channel) {
			idx = i;
			break;
		}
	}
	return idx;
}


int  initLinerarSections(Channel_t* pChannel)
{
	int rc = 0;
	
	if( pChannel) {
		pChannel->linearSections.resize(1);
		pChannel->linearSections[0].raw_value = pChannel->zero_calibration_value;
		pChannel->linearSections[0].weight_value = 0;
		pChannel->linearSections[0].ratio = 0.0;
		rc = 1;
	}
	return rc;
}


int correctionZeroLinearSections(Channel_t* pChannel)
{	
	int rc = 0;
	
	if( pChannel) {
// корректируем сырые значения		
		for( int i=0; i<(int)pChannel->linearSections.size();i++) {
			pChannel->linearSections[i].raw_value -= pChannel->diff_zero_value;
		}
		rc =  1;
	}
	
	return rc;
}

static int calcLinearRatioChannel( Channel_t* pChannel)
{
	int rc = 0;
	
	if( pChannel) {
		for( int i=0; i<(int)pChannel->linearSections.size();i++) {
			if( (i+1) < (int)pChannel->linearSections.size()) {
				pChannel->linearSections[i].ratio = 
					(float)(pChannel->linearSections[i+1].weight_value - pChannel->linearSections[i].weight_value) /
					(float)(pChannel->linearSections[i+1].raw_value-pChannel->linearSections[i].raw_value);
			}
			else if( i>0) {
				pChannel->linearSections[i].ratio = pChannel->linearSections[i-1].ratio; 
			}
		}
		rc = 1;
	}
	return rc;
}	


int setCalibrationGaneChannel( Channel_t* pChannel, int raw_value, int weight_value)
{
	int rc = 0;
	
	if( pChannel) {
		LinearSection_t new_section;
		
		for( int i=0; i<(int)pChannel->linearSections.size();i++) {
			
			// ищем линейный участок, предшедствующий указанным параметрам
			if( pChannel->linearSections[i].weight_value < weight_value)
				continue;
			
			// данный вес уже присутствует в линеаризации, то заменим raw значение и
			// пересчитаем все коэффициенты
			if( pChannel->linearSections[i].weight_value == weight_value) {
				pChannel->linearSections[i].raw_value = raw_value;
				calcLinearRatioChannel(pChannel);
				return (int)pChannel->linearSections.size();
			}
			
			new_section.weight_value = weight_value;
			new_section.raw_value = raw_value;
			new_section.ratio=0.0;
			pChannel->linearSections.insert(pChannel->linearSections.begin() + i, 1, new_section);
			calcLinearRatioChannel(pChannel);
			return (int)pChannel->linearSections.size();
		}
		
		new_section.weight_value = weight_value;
		new_section.raw_value = raw_value;
		new_section.ratio=0.0;
		pChannel->linearSections.push_back(new_section);
		
		calcLinearRatioChannel(pChannel);
		rc = (int)pChannel->linearSections.size();
	}
	return rc;
}
