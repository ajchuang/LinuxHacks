#ifndef __GPS_H__
#define __GPS_H__

/**
 * Header file of GPS location
 */

/* sent from user space */
struct gps_location {
	double latitude;
	double longitude;
	float accuracy;	/* In meter */
};

struct gps_kdata {
	unsigned char m_lat[8];	/* represent 64 bit double	*/
	unsigned char m_lon[8];	/* represent 64 bit double	*/
	unsigned char m_acc[4];	/* represent 32 bit float	*/
	unsigned char m_age[4];	/* represent 32 bit integer	*/
};

/* function prototypes */
extern void get_gps_data(struct gps_kdata *data);

#endif /* __GPS_H__ */
