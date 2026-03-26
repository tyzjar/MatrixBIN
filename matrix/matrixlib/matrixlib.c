#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "interface_matrixlib.h"
#include "typedef_matrixlib.h"

ScaleConfiguration_t scaleConfiguration;

int (*VpiFunc[VPI_MAX_FUNCTION])(VDB *);

/*
 * initialization
 */
void initialization()
{
	memset( &scaleConfiguration, 0, sizeof( scaleConfiguration));
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


int getIdxChannel( int channel)
{
	int idx = -1;
	
	if( (scaleConfiguration.cnt_channel) > 0 && scaleConfiguration.channels) {
		for( int i=0; i < scaleConfiguration.cnt_channel; i++) {
			if( scaleConfiguration.channels[i].channel == channel) {
				idx = i;
				break;
			}
		}
	}
	return idx;
}
