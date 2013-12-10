/************************************
* Photon Map Access code...
* Tweaks and tidies by Ian Stephenson
* Based on code by Henrik Jensen
* www.dctsystems.co.uk
************************************/

#ifndef PHOTONMAP_H
#define PHOTONMAP_H


/* This is the photon
 * The power is not compressed so the
 * size is 28 bytes
*/
//**********************
typedef struct Photon {
//**********************
  float pos[3];                 // photon position
  short plane;                  // splitting plane for kd-tree
  unsigned char theta, phi;     // incoming direction
  float power[3];               // photon power (uncompressed)
} Photon;


//******************************
typedef struct BalancedPhotonMap{
//******************************
  int stored_photons;
  Photon *photons;
  int half_stored_photons;
} BalancedPhotonMap;


/* This is the biggy,
 * The actual photon map structure
 */
//******************************
typedef struct PhotonMap{
//******************************
  int stored_photons;
  Photon *photons;
  int half_stored_photons;

  //The photon map MUST be the same as the balanced map up to this point.
  //What follows is only used when the map is created...
  int max_photons;
  int prev_scale;
  float bbox_min[3];		// use bbox_min;
  float bbox_max[3];		// use bbox_max;
} PhotonMap;


PhotonMap *createPhotonMap(int max_photons);
void storePhoton(PhotonMap *map,
    const float power[3],          // photon power
    const float pos[3],            // photon position
    const float dir[3]);            // photon direction

void scalePhotonPower(PhotonMap *map,
					const float scale );   // 1/(number of emitted photons)

BalancedPhotonMap *balancePhotonMap(PhotonMap *map);  // balance the kd-tree

void savePhotonMap(BalancedPhotonMap *bmap,char *filename);
BalancedPhotonMap * loadPhotonMap(char *filename);

void irradianceEstimate(
  BalancedPhotonMap *map,
  float irrad[3],                // returned irradiance
  const float pos[3],            // surface position
  const float normal[3],         // surface normal at pos
  const float max_dist,          // max distance to look for photons
  const int nphotons );     // number of photons to use
void autoIrradianceEstimate(
  BalancedPhotonMap *map,
  float irrad[3],                // returned irradiance
  const float pos[3],            // surface position
  const float normal[3],         // surface normal at pos
  const int nphotons );
void destroyPhotonMap(BalancedPhotonMap *map);

#endif // PHOTONMAP_H
