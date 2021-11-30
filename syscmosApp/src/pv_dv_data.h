#ifndef PV_DV_DATA_H
#define DV_DV_DATA_H

enum DV_Param_Type
  {
   SD_INT32,
   SD_DOUBLE,
   SD_STRING
  };

typedef struct PV_DV_Data
{
  int setter_param_num;		/* The parameter number of the setter/getter pair */
  int querier_param_num;	/* The parameter number of the querier */
  char *setter_asyn_name;	/* The string used for asyn for the setter/gett pair */
  char *querier_asyn_name;	/* The string used for asyn for the querier */
  enum DV_Param_Type data_type;	/* The type of parameter */
} PV_DV_Data_t;


#endif
