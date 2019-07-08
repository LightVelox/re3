#include "common.h"
#include "patcher.h"

#include "AudioManager.h"

#include "DMAudio.h"
#include "ModelIndices.h"
#include "MusicManager.h"
#include "Ped.h"
#include "Physical.h"
#include "PlayerPed.h"
#include "SampleManager.h"
#include "Vehicle.h"
#include "World.h"

// TODO: where is this used? Is this the right file?
enum eVehicleModel
{
	LANDSTAL,
	IDAHO,
	STINGER,
	LINERUN,
	PEREN,
	SENTINEL,
	PATRIOT,
	FIRETRUK,
	TRASH,
	STRETCH,
	MANANA,
	INFERNUS,
	BLISTA,
	PONY,
	MULE,
	CHEETAH,
	AMBULAN,
	FBICAR,
	MOONBEAM,
	ESPERANT,
	TAXI,
	KURUMA,
	BOBCAT,
	MRWHOOP,
	BFINJECT,
	CORPSE,
	POLICE,
	ENFORCER,
	SECURICA,
	BANSHEE,
	PREDATOR,
	BUS,
	RHINO,
	BARRACKS,
	TRAIN,
	CHOPPER,
	DODO,
	COACH,
	CABBIE,
	STALLION,
	RUMPO,
	RCBANDIT,
	BELLYUP,
	MRWONGS,
	MAFIA,
	YARDIE,
	YAKUZA,
	DIABLOS,
	COLUMB,
	HOODS,
	AIRTRAIN,
	DEADDODO,
	SPEEDER,
	REEFER,
	PANLANT,
	FLATBED,
	YANKEE,
	ESCAPE,
	BORGNINE,
	TOYZ,
	GHOST,
	CAR151,
	CAR152,
	CAR153,
	CAR154,
	CAR155,
	CAR156,
	CAR157,
	CAR158,
	CAR159,
};

cAudioManager &AudioManager = *(cAudioManager *)0x880FC0;

constexpr int totalAudioEntitiesSlots = 200;
constexpr int maxVolume = 127;

char &g_nMissionAudioPlayingStatus = *(char *)0x60ED88;

void
cAudioManager::AddSampleToRequestedQueue()
{
	int32 calculatedVolume;
	tActiveSample *sample;
	int32 unknown1;
	uint8 unknown2;
	bool bReflections;

	if(m_sQueueSample.m_nSampleIndex < TOTAL_AUDIO_SAMPLES) {
		calculatedVolume = m_sQueueSample.field_16 * (maxVolume - m_sQueueSample.m_bVolume);
		unknown2 = m_bSampleRequestQueuesStatus[m_bActiveSampleQueue];
		if(unknown2 >= m_bActiveSamples) {
			unknown1 = 27 * m_bActiveSampleQueue;
			unknown2 = *(&m_asSamples[53].field_91 + m_bActiveSamples + unknown1);
			if(m_asSamples[unknown1 + unknown2].calculatedVolume <= calculatedVolume)
				return;
		} else {
			++m_bSampleRequestQueuesStatus[m_bActiveSampleQueue];
		}
		m_sQueueSample.calculatedVolume = calculatedVolume;
		m_sQueueSample.m_bLoopEnded = 0;
		if(m_sQueueSample.m_bIsDistant) {
			m_sQueueSample.m_bRequireReflection = 0;
			m_sQueueSample.m_bLoopsRemaining = 0;
		}
		if(m_bDynamicAcousticModelingStatus && m_sQueueSample.m_nLoopCount) {
			bReflections = m_sQueueSample.m_bRequireReflection;
		} else {
			bReflections = false;
			m_sQueueSample.m_bLoopsRemaining = 0;
		}
		m_sQueueSample.m_bRequireReflection = 0;

		if(!m_bDynamicAcousticModelingStatus) m_sQueueSample.m_bReverbFlag = 0;

		sample = &m_asSamples[27 * m_bActiveSampleQueue + unknown2];
		sample->m_nEntityIndex = m_sQueueSample.m_nEntityIndex;
		sample->field_4 = m_sQueueSample.field_4;
		sample->m_nSampleIndex = m_sQueueSample.m_nSampleIndex;
		sample->m_bBankIndex = m_sQueueSample.m_bBankIndex;
		sample->m_bIsDistant = m_sQueueSample.m_bIsDistant;
		sample->field_16 = m_sQueueSample.field_16;
		sample->m_nFrequency = m_sQueueSample.m_nFrequency;
		sample->m_bVolume = m_sQueueSample.m_bVolume;
		sample->m_fDistance = m_sQueueSample.m_fDistance;
		sample->m_nLoopCount = m_sQueueSample.m_nLoopCount;
		sample->m_nLoopStart = m_sQueueSample.m_nLoopStart;
		sample->m_nLoopEnd = m_sQueueSample.m_nLoopEnd;
		sample->m_bEmittingVolume = m_sQueueSample.m_bEmittingVolume;
		sample->field_48 = m_sQueueSample.field_48;
		sample->m_fSoundIntensity = m_sQueueSample.m_fSoundIntensity;
		sample->field_56 = m_sQueueSample.field_56;
		sample->m_vecPos = m_sQueueSample.m_vecPos;
		sample->m_bReverbFlag = m_sQueueSample.m_bReverbFlag;
		sample->m_bLoopsRemaining = m_sQueueSample.m_bLoopsRemaining;
		sample->m_bRequireReflection = m_sQueueSample.m_bRequireReflection;
		sample->m_bOffset = m_sQueueSample.m_bOffset;
		sample->field_76 = m_sQueueSample.field_76;
		sample->m_bIsProcessed = m_sQueueSample.m_bIsProcessed;
		sample->m_bLoopEnded = m_sQueueSample.m_bLoopEnded;
		sample->calculatedVolume = m_sQueueSample.calculatedVolume;
		sample->field_88 = m_sQueueSample.field_88;

		AddDetailsToRequestedOrderList(unknown2);
		if(bReflections) AddReflectionsToRequestedQueue();
	}
}

void
cAudioManager::AddDetailsToRequestedOrderList(uint8 sample)
{
	int32 offset;
	uint32 i = 0;
	if(sample != 0) {
		for(; i < sample; i++) {
			offset = 27 * m_bActiveSampleQueue;
			if(m_asSamples[offset + m_abSampleQueueIndexTable[i + offset]]
			       .calculatedVolume > m_asSamples[offset + sample].calculatedVolume)
				break;
		}
		if(i < sample) {
			memmove(&m_abSampleQueueIndexTable[offset + 1 + i],
			        &m_abSampleQueueIndexTable[offset + i], m_bActiveSamples - i - 1);
		}
	}
	m_abSampleQueueIndexTable[27 * m_bActiveSampleQueue + i] = sample;
}

void
cAudioManager::AddReflectionsToRequestedQueue()
{
	float reflectionDistance;
	int32 noise;
	uint8 emittingVolume = emittingVolume =
	    (m_sQueueSample.m_bVolume >> 1) + (m_sQueueSample.m_bVolume >> 3);

	for(uint32 i = 0; i < 5u; i++) {
		reflectionDistance = m_afReflectionsDistances[i];
		if(reflectionDistance > 0.0f && reflectionDistance < 100.f &&
		   reflectionDistance < m_sQueueSample.m_fSoundIntensity) {
			m_sQueueSample.m_bLoopsRemaining = (reflectionDistance * 0.38873f); // @todo assert value
			if(m_sQueueSample.m_bLoopsRemaining > 5u) {
				m_sQueueSample.m_fDistance = m_afReflectionsDistances[i];
				m_sQueueSample.m_bEmittingVolume = emittingVolume;
				m_sQueueSample.m_bVolume =
				    ComputeVolume(emittingVolume, m_sQueueSample.m_fSoundIntensity,
				                  m_sQueueSample.m_fDistance);
				if(m_sQueueSample.m_bVolume > emittingVolume >> 4) {
					m_sQueueSample.field_4 += ((i + 1) << 8);
					if(m_sQueueSample.m_nLoopCount) {
						noise = RandomDisplacement(
						    m_sQueueSample.m_nFrequency >> 5);
						if(noise <= 0)
							m_sQueueSample.m_nFrequency += noise;
						else
							m_sQueueSample.m_nFrequency -= noise;
					}
					m_sQueueSample.field_16 += 20;
					m_sQueueSample.m_vecPos.x = m_avecReflectionsPos[i].x;
					m_sQueueSample.m_vecPos.y = m_avecReflectionsPos[i].y;
					m_sQueueSample.m_vecPos.z = m_avecReflectionsPos[i].z;
					AddSampleToRequestedQueue();
				}
			}
		}
	}
}

uint32
cAudioManager::ComputeVolume(int emittingVolume, float soundIntensity, float distance)
{
	float newSoundIntensity;
	if(soundIntensity <= 0.0f) return 0;
	if((soundIntensity * 0.2f) <= distance) {
		newSoundIntensity = soundIntensity * 0.2f;
		emittingVolume =
		    sq((soundIntensity - distance) / (soundIntensity - newSoundIntensity)) *
		    emittingVolume;
	}
	return emittingVolume;
}

void
cAudioManager::Initialise()
{
	if(!m_bIsInitialised) {
		PreInitialiseGameSpecificSetup();
		m_bIsInitialised = cSampleManager.Initialise();
		if(m_bIsInitialised) {
			m_bActiveSamples = cSampleManager.GetActiveSamples();
			if(m_bActiveSamples <= 1u) {
				Terminate();
			} else {
				--m_bActiveSamples;
				PostInitialiseGameSpecificSetup();
				InitialisePoliceRadioZones();
				InitialisePoliceRadio();
				MusicManager.Initialise();
			}
		}
	}
}

void
cAudioManager::PostInitialiseGameSpecificSetup()
{
	m_nFireAudioEntity = CreateEntity(
	    AUDIOTYPE_FIRE, (CPhysical *)0x8F31D0); // last is addr of firemanager @todo change
	if(m_nFireAudioEntity >= 0) cAudioManager::SetEntityStatus(m_nFireAudioEntity, 1);

	m_nCollisionEntity = CreateEntity(AUDIOTYPE_COLLISION, (CPhysical *)1);
	if(m_nCollisionEntity >= 0) cAudioManager::SetEntityStatus(m_nCollisionEntity, 1);

	m_nFrontEndEntity = CreateEntity(AUDIOTYPE_FRONTEND, (CPhysical *)1);
	if(m_nFrontEndEntity >= 0) cAudioManager::SetEntityStatus(m_nFrontEndEntity, 1);

	m_nProjectileEntity = CreateEntity(AUDIOTYPE_PROJECTILE, (CPhysical *)1);
	if(m_nProjectileEntity >= 0) cAudioManager::SetEntityStatus(m_nProjectileEntity, 1);

	m_nWaterCannonEntity = CreateEntity(AUDIOTYPE_WATER_CANNON, (CPhysical *)1);
	if(m_nWaterCannonEntity >= 0) cAudioManager::SetEntityStatus(m_nWaterCannonEntity, 1);

	m_nPoliceChannelEntity = CreateEntity(AUDIOTYPE_D, (CPhysical *)1);
	if(m_nPoliceChannelEntity >= 0) cAudioManager::SetEntityStatus(m_nPoliceChannelEntity, 1);

	m_nBridgeEntity = CreateEntity(AUDIOTYPE_BRIDGE, (CPhysical *)1);
	if(m_nBridgeEntity >= 0) cAudioManager::SetEntityStatus(m_nBridgeEntity, 1);

	m_sMissionAudio.m_nSampleIndex = NO_SAMPLE;
	m_sMissionAudio.m_bLoadingStatus = 0;
	m_sMissionAudio.m_bPlayStatus = 0;
	m_sMissionAudio.field_22 = 0;
	m_sMissionAudio.m_bIsPlayed = 0;
	m_sMissionAudio.field_12 = 1;
	m_sMissionAudio.field_24 = 0;
	ResetAudioLogicTimers((int32)CTimer::GetTimeInMilliseconds);
}

WRAPPER
void
cAudioManager::InitialisePoliceRadioZones()
{
	EAXJMP(0x57EAC0);
}

WRAPPER
void
cAudioManager::ResetAudioLogicTimers(int32 timer)
{
	EAXJMP(0x569650);
}

void
cAudioManager::Terminate()
{
	if(m_bIsInitialised) {
		MusicManager.Terminate();

		for(uint32 i = 0; i < totalAudioEntitiesSlots; i++) {
			m_asAudioEntities[i].m_bIsUsed = 0;
			m_anAudioEntityIndices[i] = 200;
		}

		m_nAudioEntitiesTotal = 0;
		m_nScriptObjectEntityTotal = 0;
		PreTerminateGameSpecificShutdown();

		for(uint32 i = 0; i < 2; i++) {
			if(cSampleManager.IsSampleBankLoaded(i)) cSampleManager.UnloadSampleBank(i);
		}

		cSampleManager.Terminate();

		m_bIsInitialised = 0;
		PostTerminateGameSpecificShutdown();
	}
}

char
cAudioManager::GetMissionScriptPoliceAudioPlayingStatus()
{
	return g_nMissionAudioPlayingStatus;
}

bool
cAudioManager::GetMissionAudioLoadingStatus()
{
	if(m_bIsInitialised) return m_sMissionAudio.m_bLoadingStatus;

	return true;
}


uint8
cAudioManager::GetNum3DProvidersAvailable()
{
	if(m_bIsInitialised) return num3DProvidersAvailable;
	return 0;
}

int8
cAudioManager::AutoDetect3DProviders()
{
	if(m_bIsInitialised) return cSampleManager.AutoDetect3DProviders();

	return -1;
}

bool
cAudioManager::IsMP3RadioChannelAvailable()
{
	if(m_bIsInitialised) return cSampleManager.IsMP3RadioChannelAvailable();

	return 0;
}

uint8
cAudioManager::GetCDAudioDriveLetter()
{
	if(m_bIsInitialised) return cSampleManager.GetCDAudioDriveLetter();

	return 0;
}

void
cAudioManager::SetEffectsMasterVolume(uint8 volume)
{
	cSampleManager.SetEffectsMasterVolume(volume);
}

void
cAudioManager::SetMusicMasterVolume(uint8 volume)
{
	cSampleManager.SetMusicMasterVolume(volume);
}

void
cAudioManager::SetEffectsFadeVol(uint8 volume)
{
	cSampleManager.SetEffectsFadeVol(volume);
}

void
cAudioManager::SetMusicFadeVol(uint8 volume)
{
	cSampleManager.SetMusicFadeVol(volume);
}

void
cAudioManager::SetSpeakerConfig(int32 conf)
{
	cSampleManager.SetSpeakerConfig(conf);
}

void
cAudioManager::ProcessJumboFlying()
{
	if(SetupJumboFlySound(127u)) SetupJumboEngineSound(63u, 22050);
}

WRAPPER
bool cAudioManager::SetupJumboEngineSound(uint8, int32) { EAXJMP(0x56F140); }

int32 *BankStartOffset = (int32 *)0x6FAB70; //[2]

void
cAudioManager::PreInitialiseGameSpecificSetup()
{
	BankStartOffset[0] = AUDIO_SAMPLE_VEHICLE_HORN_0;
	BankStartOffset[1] = AUDIO_SAMPLE_POLICE_COP_1_ARREST_1;
}

int32 &g_nMissionAudioSfx = *(int32 *)0x60ED84;

void
cAudioManager::SetMissionScriptPoliceAudio(int32 sfx)
{
	if(m_bIsInitialised) {
		if(g_nMissionAudioPlayingStatus != 1) {
			g_nMissionAudioPlayingStatus = 0;
			g_nMissionAudioSfx = sfx;
		}
	}
}

void
cAudioManager::CalculateDistance(bool *ptr, float dist)
{
	if(*ptr == false) {
		m_sQueueSample.m_fDistance = sqrt(dist);
		*ptr = true;
	}
}

bool
cAudioManager::UsesSiren(int32 model)
{
	switch(model) {
	case FIRETRUK:
	case AMBULAN:
	case FBICAR:
	case POLICE:
	case ENFORCER:
	case PREDATOR: return true;
	default: return false;
	}
}

bool
cAudioManager::UsesSirenSwitching(int32 model)
{
	switch(model) {
	case AMBULAN:
	case POLICE:
	case ENFORCER:
	case PREDATOR: return true;
	default: return false;
	}
}

bool
cAudioManager::MissionScriptAudioUsesPoliceChannel(int32 soundMission)
{
	switch(soundMission) {
	case STREAMED_SOUND_MISSION_J6_D:
	case STREAMED_SOUND_MISSION_T4_A:
	case STREAMED_SOUND_MISSION_S1_H:
	case STREAMED_SOUND_MISSION_S3_B:
	case STREAMED_SOUND_MISSION_EL3_A:
	case STREAMED_SOUND_MISSION_A3_A:
	case STREAMED_SOUND_MISSION_A5_A:
	case STREAMED_SOUND_MISSION_K1_A:
	case STREAMED_SOUND_MISSION_R1_A:
	case STREAMED_SOUND_MISSION_R5_A:
	case STREAMED_SOUND_MISSION_LO2_A:
	case STREAMED_SOUND_MISSION_LO6_A: return true;
	default: return false;
	}
}

uint8
cAudioManager::Get3DProviderName(uint8 id)
{
	if(m_bIsInitialised) return 0;
	if(id >= num3DProvidersAvailable) return 0;
	return asName3DProviders[id];
}

void
cAudioManager::ProcessJumboTaxi()
{
	if(SetupJumboFlySound(20u)) {
		if(SetupJumboTaxiSound(75u)) SetupJumboWhineSound(18u, 29500);
	}
}

WRAPPER
bool cAudioManager::SetupJumboFlySound(uint8) { EAXJMP(0x56F230); }

WRAPPER
bool cAudioManager::SetupJumboTaxiSound(uint8) { EAXJMP(0x56EF20); }

WRAPPER
bool cAudioManager::SetupJumboWhineSound(uint8, int32) { EAXJMP(0x56F070); }

void
cAudioManager::PlayLoadedMissionAudio()
{
	if(m_bIsInitialised && m_sMissionAudio.m_nSampleIndex != NO_SAMPLE &&
	   m_sMissionAudio.m_bLoadingStatus == 1 && !m_sMissionAudio.m_bPlayStatus) {
		m_sMissionAudio.m_bIsPlayed = true;
	}
}

void
cAudioManager::SetMissionAudioLocation(float x, float y, float z)
{
	if(m_bIsInitialised) {
		m_sMissionAudio.field_12 = 0;
		m_sMissionAudio.m_vecPos.x = x;
		m_sMissionAudio.m_vecPos.y = y;
		m_sMissionAudio.m_vecPos.z = z;
	}
}

void
cAudioManager::ResetPoliceRadio()
{
	if(m_bIsInitialised) {
		if(cSampleManager.GetChannelUsedFlag(28)) cSampleManager.StopChannel(28);
		InitialisePoliceRadio();
	}
}

void
cAudioManager::InterrogateAudioEntities()
{
	for(int32 i = 0; i < m_nAudioEntitiesTotal; i++) {
		ProcessEntity(m_anAudioEntityIndices[i]);
		m_asAudioEntities[m_anAudioEntityIndices[i]].field_24 = 0;
	}
}

void
cAudioManager::ClearRequestedQueue()
{
	for(int32 i = 0; i < m_bActiveSamples; i++) {
		m_abSampleQueueIndexTable[i + 27 * m_bActiveSampleQueue] = m_bActiveSamples;
	}
	m_bSampleRequestQueuesStatus[m_bActiveSampleQueue] = 0;
}

// void cAudioManager::AgeCrimes()
//{

//}

bool
cAudioManager::UsesReverseWarning(int32 model)
{
	return model == LINERUN || model == FIRETRUK || model == TRASH || model == BUS ||
	       model == COACH; // fix
}

bool
cAudioManager::HasAirBrakes(int32 model)
{
	return model == LINERUN || model == FIRETRUK || model == TRASH || model == BUS ||
	       model == COACH; // fix
}

int32
cAudioManager::GetJumboTaxiFreq()
{
	return (60.833f * m_sQueueSample.m_fDistance) + 22050;
}

void
cAudioManager::ProcessPhysical(int32 id)
{
	CPhysical *entity = m_asAudioEntities[id].m_pEntity;
	if(entity) {
		switch(entity->m_type & 7) {
		case ENTITY_TYPE_VEHICLE: ProcessVehicle(m_asAudioEntities[id].m_pEntity); break;
		case ENTITY_TYPE_PED: ProcessPed(m_asAudioEntities[id].m_pEntity); break;
		default: return;
		}
	}
}

WRAPPER
void
cAudioManager::ProcessVehicle(CPhysical *)
{
	EAXJMP(0x569A00);
}

WRAPPER
void
cAudioManager::ProcessPed(CPhysical *)
{
	EAXJMP(0x56F450);
}

WRAPPER
void
cAudioManager::ProcessPlane(void *ptr)
{
	EAXJMP(0x56E860);
}

void
cAudioManager::ClearMissionAudio()
{
	if(m_bIsInitialised) {
		m_sMissionAudio.m_nSampleIndex = NO_SAMPLE;
		m_sMissionAudio.m_bLoadingStatus = 0;
		m_sMissionAudio.m_bPlayStatus = 0;
		m_sMissionAudio.field_22 = 0;
		m_sMissionAudio.m_bIsPlayed = false;
		m_sMissionAudio.field_12 = 1;
		m_sMissionAudio.field_24 = 0;
	}
}

// void
// cAudioManager::ProcessReverb()
//{
//}

bool
cAudioManager::IsMissionAudioSampleFinished()
{
	if(m_bIsInitialised) return m_sMissionAudio.m_bPlayStatus == 2;

	static int32 cPretendFrame = 1;

	return (cPretendFrame++ & 63) == 0;
}

WRAPPER
void cAudioManager::ProcessEntity(int32) { EAXJMP(0x569870); }

WRAPPER
void
cAudioManager::InitialisePoliceRadio()
{
	EAXJMP(0x57EEC0);
}

int32
cAudioManager::RandomDisplacement(uint32 seed)
{
	int32 value;

	static bool bIsEven = true;
	static uint32 base = 0;

	if(!seed) return 0;

	value = m_anRandomTable[(base + seed) % 5] % seed;
	base += value;

	if(value % 2) { bIsEven = !bIsEven; }
	if(!bIsEven) value = -value;
	return value;
}

void
cAudioManager::ReleaseDigitalHandle()
{
	if(m_bIsInitialised) { cSampleManager.ReleaseDigitalHandle(); }
}

void
cAudioManager::RequireDigitalHandle()
{
	if(m_bIsInitialised) { cSampleManager.RequireDigitalHandle(); }
}

void
cAudioManager::SetDynamicAcousticModelingStatus(bool status)
{
	m_bDynamicAcousticModelingStatus = status;
}

bool
cAudioManager::IsAudioInitialised() const
{
	return m_bIsInitialised;
}

int32
cAudioManager::CreateEntity(int32 type, CPhysical *entity)
{
	if(!m_bIsInitialised) return -4;
	if(!entity) return -2;
	if(type >= TOTAL_AUDIO_TYPES) return -1;
	for(uint32 i = 0; i < 200; i++) {
		if(!m_asAudioEntities[i].m_bIsUsed) {
			m_asAudioEntities[i].m_bIsUsed = true;
			m_asAudioEntities[i].m_bStatus = 0;
			m_asAudioEntities[i].m_nType = (eAudioType)type;
			m_asAudioEntities[i].m_pEntity = entity;
			m_asAudioEntities[i].m_awAudioEvent[0] = SOUND_TOTAL_PED_SOUNDS;
			m_asAudioEntities[i].m_awAudioEvent[1] = SOUND_TOTAL_PED_SOUNDS;
			m_asAudioEntities[i].m_awAudioEvent[2] = SOUND_TOTAL_PED_SOUNDS;
			m_asAudioEntities[i].m_awAudioEvent[3] = SOUND_TOTAL_PED_SOUNDS;
			m_asAudioEntities[i].field_24 = 0;
			m_anAudioEntityIndices[m_nAudioEntitiesTotal++] = i;
			return i;
		}
	}
	return -3;
}

void
cAudioManager::DestroyEntity(int32 id)
{
	if(m_bIsInitialised && id >= 0 && id < totalAudioEntitiesSlots &&
	   m_asAudioEntities[id].m_bIsUsed) {
		m_asAudioEntities[id].m_bIsUsed = 0;
		for(int32 i = 0; i < m_nAudioEntitiesTotal; ++i) {
			if(id == m_anAudioEntityIndices[i]) {
				if(i < totalAudioEntitiesSlots - 1)
					memmove(&m_anAudioEntityIndices[i],
					        &m_anAudioEntityIndices[i + 1],
					        4 * (m_nAudioEntitiesTotal - (i + 1)));
				m_anAudioEntityIndices[--m_nAudioEntitiesTotal] =
				    totalAudioEntitiesSlots;
				return;
			}
		}
	}
}

void
cAudioManager::SetEntityStatus(int32 id, bool status)
{
	if(m_bIsInitialised && id >= 0 && id < totalAudioEntitiesSlots) {
		if(m_asAudioEntities[id].m_bIsUsed) { m_asAudioEntities[id].m_bStatus = status; }
	}
}

void
cAudioManager::PreTerminateGameSpecificShutdown()
{
	if(m_nBridgeEntity >= 0) {
		DestroyEntity(m_nBridgeEntity);
		m_nBridgeEntity = -5;
	}
	if(m_nPoliceChannelEntity >= 0) {
		DestroyEntity(m_nPoliceChannelEntity);
		m_nPoliceChannelEntity = -5;
	}
	if(m_nWaterCannonEntity >= 0) {
		DestroyEntity(m_nWaterCannonEntity);
		m_nWaterCannonEntity = -5;
	}
	if(m_nFireAudioEntity >= 0) {
		DestroyEntity(m_nFireAudioEntity);
		m_nFireAudioEntity = -5;
	}
	if(m_nCollisionEntity >= 0) {
		DestroyEntity(m_nCollisionEntity);
		m_nCollisionEntity = -5;
	}
	if(m_nFrontEndEntity >= 0) {
		DestroyEntity(m_nFrontEndEntity);
		m_nFrontEndEntity = -5;
	}
	if(m_nProjectileEntity >= 0) {
		DestroyEntity(m_nProjectileEntity);
		m_nProjectileEntity = -5;
	}
}

void
cAudioManager::PostTerminateGameSpecificShutdown()
{
	;
}

void
cAudioManager::GenerateIntegerRandomNumberTable()
{
	for(int32 i = 0; i < 5; i++) { m_anRandomTable[i] = rand(); }
}

bool &bPlayerJustEnteredCar = *(bool *)0x6508C4;

void
cAudioManager::PlayerJustGotInCar()
{
	if(m_bIsInitialised) { bPlayerJustEnteredCar = true; }
}

void
cAudioManager::PlayerJustLeftCar(void)
{
	// UNUSED: This is a perfectly empty function.
}

void
cAudioManager::GetPhrase(uint32 *phrase, uint32 *prevPhrase, uint32 sample, uint32 maxOffset)
{
	*phrase = sample + m_anRandomTable[m_sQueueSample.m_nEntityIndex & 3] % maxOffset;

	// check if the same sfx like last time, if yes, then try use next one,
	// if exceeded range, then choose first available sample
	if(*phrase == *prevPhrase && ++*phrase >= sample + maxOffset) *phrase = sample;
	*prevPhrase = *phrase;
}

uint32
cAudioManager::GetPlayerTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_DAMAGE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DAMAGE_REACTION_1, 11u);
		break;

	case SOUND_PED_HIT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HIT_REACTION_1, 10u); break;

	case SOUND_PED_LAND: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FALL_REACTION_1, 6u); break;

	default: sfx = NO_SAMPLE; break;
	}
	return sfx;
}

uint32
cAudioManager::GetCopTalkSfx(int16 sound)
{
	uint32 sfx;
	PedState pedState;
	static uint32 lastSfx = NO_SAMPLE;

	if(sound == SOUND_PED_ARREST_COP) {
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_POLICE_COP_1_ARREST_1, 6u);
	} else {
		if(sound != SOUND_PED_PURSUIT_COP) {
			return cAudioManager::GetGenericMaleTalkSfx(sound);
		}

		pedState = FindPlayerPed()->m_nPedState;
		if(pedState == PED_ARRESTED || pedState == PED_DEAD || pedState == PED_DIE)
			return NO_SAMPLE;
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_POLICE_COP_1_PURSUIT_1, 7u);
	}

	return 13 * (m_sQueueSample.m_nEntityIndex % 5) + sfx;
}

uint32
cAudioManager::GetSwatTalkSfx(int16 sound)
{
	uint32 sfx;
	PedState pedState;
	static uint32 lastSfx = NO_SAMPLE;

	if(sound == SOUND_PED_ARREST_SWAT) {
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_POLICE_SWAT_1_PURSUIT_ARREST_1, 6u);
	} else {
		if(sound != SOUND_PED_PURSUIT_SWAT) {
			return cAudioManager::GetGenericMaleTalkSfx(sound);
		}

		pedState = FindPlayerPed()->m_nPedState;
		if(pedState == PED_ARRESTED || pedState == PED_DEAD || pedState == PED_DIE)
			return NO_SAMPLE;
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_POLICE_SWAT_1_PURSUIT_ARREST_1, 6u);
	}

	return 6 * (m_sQueueSample.m_nEntityIndex % 3) + sfx;
}

uint32
cAudioManager::GetFBITalkSfx(int16 sound)
{
	uint32 sfx;
	PedState pedState;
	static uint32 lastSfx = NO_SAMPLE;

	if(sound == SOUND_PED_ARREST_FBI) {
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_POLICE_FBI_1_PURSUIT_ARREST_1, 6u);
	} else {
		if(sound != SOUND_PED_PURSUIT_FBI) {
			return cAudioManager::GetGenericMaleTalkSfx(sound);
		}

		pedState = FindPlayerPed()->m_nPedState;
		if(pedState == PED_ARRESTED || pedState == PED_DEAD || pedState == PED_DIE)
			return NO_SAMPLE;
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_POLICE_FBI_1_PURSUIT_ARREST_1, 6u);
	}

	return 6 * (m_sQueueSample.m_nEntityIndex % 3) + sfx;
}

uint32
cAudioManager::GetArmyTalkSfx(int16 sound)
{
	uint32 sfx;
	PedState pedState;
	static uint32 lastSfx = NO_SAMPLE;

	if(sound != SOUND_PED_PURSUIT_ARMY) { return cAudioManager::GetGenericMaleTalkSfx(sound); }

	pedState = FindPlayerPed()->m_nPedState;
	if(pedState == PED_ARRESTED || pedState == PED_DEAD || pedState == PED_DIE)
		return NO_SAMPLE;
	GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_POLICE_ARMY_1_PURSUIT_1, 15u);

	return 15 * (m_sQueueSample.m_nEntityIndex % 1) + sfx;
}

uint32
cAudioManager::GetMedicTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MEDIC_1_HANDS_COWER_1, 5u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MEDIC_1_CAR_JACKED_1, 5u);
		break;
	case SOUND_PED_HEALING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MEDIC_1_HEALING_1, 12u);
		break;
	case SOUND_PED_LEAVE_VEHICLE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MEDIC_1_LEAVE_VEHICLE_1, 9u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MEDIC_1_FLEE_RUN_1, 6u);
		break;
	default: return cAudioManager::GetGenericMaleTalkSfx(sound);
	}
	return 37 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetFiremanTalkSfx(int16 sound)
{
	return cAudioManager::GetGenericMaleTalkSfx(sound);
}

uint32
cAudioManager::GetNormalMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_HANDS_COWER_1, 7u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_CAR_JACKED_1, 7u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_EVADE_1, 9u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_FLEE_RUN_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_CAR_COLLISION_1, 12u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_CHAT_SEXY_1, 8u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_CHAT_EVENT_1, 0xAu);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_NORMAL_MALE_CHAT_1, 25u);
		break;
	default: return cAudioManager::GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetTaxiDriverTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	if(sound == SOUND_PED_CAR_JACKED) {
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TAXI_DRIVER_1_CAR_JACKED_1, 7u);
	} else {
		if(sound != SOUND_PED_CAR_COLLISION)
			return cAudioManager::GetGenericMaleTalkSfx(sound);
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TAXI_DRIVER_1_CAR_COLLISION_1, 6u);
	}
	return 13 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetPimpTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_PIMP_HANDS_UP_1, 7u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_PIMP_CAR_JACKED_1, 4u);
		break;
	case SOUND_PED_DEFEND: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_PIMP_DEFEND_1, 9u); break;
	case SOUND_PED_EVADE: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_PIMP_EVADE_1, 6u); break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_PIMP_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_PIMP_CHAT_EVENT_1, 2u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_PIMP_CHAT_1, 17u); break;
	default: return cAudioManager::GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetMafiaTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MAFIA_1_CAR_JACKING_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MAFIA_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MAFIA_1_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MAFIA_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MAFIA_1_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MAFIA_1_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MAFIA_1_CHAT_1, 7u); break;
	default: return cAudioManager::GetGenericMaleTalkSfx(sound);
	}
	return 30 * (m_sQueueSample.m_nEntityIndex % 3) + sfx;
}

uint32
cAudioManager::GetTriadTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_HANDS_UP_1, 3u);
		break;
	case SOUND_PED_CAR_JACKING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_CAR_JACKING_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_CAR_COLLISION_1, 7u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_TRIAD_1_CHAT_1, 8u); break;
	default: return cAudioManager::GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetDiabloTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_HANDS_UP_1, 4u);
		break;
	case SOUND_PED_HANDS_COWER:
		sound = SOUND_PED_FLEE_SPRINT;
		return cAudioManager::GetGenericMaleTalkSfx(sound);
		break;
	case SOUND_PED_CAR_JACKING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_CAR_JACKING_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_CHAT_SEXY_1, 4u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_DIABLO_1_CHAT_1, 5u); break;
	default: return cAudioManager::GetGenericMaleTalkSfx(sound);
	}
	return 30 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetYakuzaTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YAKUZA_1_CAR_JACKING_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YAKUZA_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YAKUZA_1_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YAKUZA_1_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YAKUZA_1_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YAKUZA_1_CHAT_1, 5u); break;
	default: return cAudioManager::GetGenericMaleTalkSfx(sound);
	}
	return 24 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetYardieTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP: sfx = AUDIO_SAMPLE_PED_YARDIE_1_HANDS_UP_1; break;
	case SOUND_PED_CAR_JACKING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YARDIE_1_CAR_JACKING_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED: sfx = AUDIO_SAMPLE_PED_YARDIE_1_CAR_JACKED_1; break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YARDIE_1_ATTACK_1, 6u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YARDIE_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YARDIE_1_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YARDIE_1_CHAT_SEXY_1, 2u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_YARDIE_1_CHAT_1, 8u); break;
	default: return cAudioManager::GetGenericMaleTalkSfx(sound);
	}
	return 31 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetColumbianTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_COLUMB_1_CAR_JACKING_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_COLUMB_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_COLUMB_1_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_COLUMB_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_COLUMB_1_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_COLUMB_1_CHAT_SEXY_1, 2u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_COLUMB_1_CHAT_1, 5u); break;
	default: return cAudioManager::GetGenericMaleTalkSfx(sound);
	}
	return 27 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetHoodTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_HANDS_UP_1, 5u);
		break;
	case SOUND_PED_CAR_JACKING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_CAR_JACKING_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_ATTACK_1, 6u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_CAR_COLLISION_1, 7u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_CHAT_SEXY_1, 2u);
		break;
	case SOUND_PED_CHAT: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOODS_1_CHAT_1, 6u); break;

	default: return cAudioManager::GetGenericMaleTalkSfx(sound); break;
	}
	return 35 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetBlackCriminalTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CRIMINAL_1_HANDS_UP_1, 4u);
		break;
	case SOUND_PED_CAR_JACKING: sfx = AUDIO_SAMPLE_PED_BLACK_CRIMINAL_1_CAR_JACKING_1; break;
	case SOUND_PED_MUGGING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CRIMINAL_1_MUGGING_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CRIMINAL_1_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CRIMINAL_1_EVADE_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CRIMINAL_1_CAR_COLLISION_1, 5u);
		break;
	default: return cAudioManager::GetGenericMaleTalkSfx(sound); break;
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteCriminalTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CRIMINAL_1_HANDS_UP_1, 3u);
		break;
	case SOUND_PED_CAR_JACKING: sfx = AUDIO_SAMPLE_PED_WHITE_CRIMINAL_1_CAR_JACKING_1; break;
	case SOUND_PED_MUGGING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CRIMINAL_1_MUGGING_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CRIMINAL_1_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CRIMINAL_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CRIMINAL_1_CAR_COLLISION_1, 4u);
		break;
	default: return cAudioManager::GetGenericMaleTalkSfx(sound); break;
	}
	return sfx;
}

uint32
cAudioManager::GetMaleNo2TalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		cAudioManager::GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_NO_2_CAR_JACKED_1,
		                         3u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_NO_2_ROBBED_1, 4u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_NO_2_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_NO_2_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		cAudioManager::GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_NO_2_CAR_COLLISION_1,
		                         7u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_NO_2_CHAT_SEXY_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_NO_2_CHAT_1, 7u);
		break;
	default: return cAudioManager::GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackProjectMaleTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_HANDS_UP_1, 3u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_ATTACK_1, 6u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_CAR_COLLISION_1,
		          7u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_MALE_1_CHAT_1, 6u);
	default: return GetGenericMaleTalkSfx(sound);
	}

	if(model == MI_P_MAN2) sfx += 34;
	return sfx;
}

uint32
cAudioManager::GetWhiteFatMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_MALE_1_CAR_JACKED_1, 3u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_MALE_1_ROBBED_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_MALE_1_EVADE_1, 9u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_MALE_1_CAR_COLLISION_1, 9u);
		break;
	case SOUND_PED_WAIT_DOUBLEBACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_MALE_1_WAIT_DOUBLEBACK_1, 2u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_MALE_1_CHAT_1, 9u);
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackFatMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_MALE_1_CAR_JACKED_1, 4u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_MALE_1_ROBBED_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_MALE_1_EVADE_1, 7u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_MALE_1_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_WAIT_DOUBLEBACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_MALE_1_WAIT_DOUBLEBACK_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_MALE_1_CHAT_1, 8u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackCasualFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_HANDS_COWER_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_ROBBED_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_EVADE_1, 6u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_FLEE_RUN_1, 2u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_CAR_COLLISION_1,
		          7u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_CHAT_EVENT_1, 4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CASUAL_FEMALE_1_CHAT_1, 8u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteCasualFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_HANDS_COWER_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED: sfx = AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_ROBBED_1; break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_EVADE_1, 3u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_FLEE_RUN_1, 2u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_CAR_COLLISION_1,
		          8u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_CHAT_EVENT_1, 2u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CASUAL_FEMALE_1_CHAT_1, 4u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetFemaleNo3TalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_HANDS_COWER_1, 5u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_CAR_JACKED_1, 3u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_ROBBED_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_EVADE_1, 6u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_FLEE_RUN_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_CHAT_EVENT_1, 4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_NO_3_CHAT_1, 5u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackFatFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_FEMALE_1_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_FEMALE_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_FEMALE_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_FEMALE_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_FEMALE_1_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_FEMALE_1_CHAT_EVENT_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FAT_FEMALE_1_CHAT_1, 7u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteFatFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_FEMALE_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_FEMALE_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_FEMALE_1_EVADE_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_FEMALE_1_CAR_COLLISION_1, 8u);
		break;
	case SOUND_PED_WAIT_DOUBLEBACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_FEMALE_1_WAIT_DOUBLEBACK_1,
		          2u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_FEMALE_1_CHAT_EVENT_1, 4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FAT_FEMALE_1_CHAT_1, 8u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackFemaleProstituteTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FEMALE_PROSTITUTE_1_HANDS_UP_1,
		          4u);
		break;
	case SOUND_PED_ROBBED: sfx = AUDIO_SAMPLE_PED_BLACK_FEMALE_PROSTITUTE_1_ROBBED_1; break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FEMALE_PROSTITUTE_1_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FEMALE_PROSTITUTE_1_EVADE_1, 3u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_BLACK_FEMALE_PROSTITUTE_1_CAR_COLLISION_1, 4u);
		break;
	case SOUND_PED_SOLICIT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FEMALE_PROSTITUTE_1_SOLICIT_1, 8u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_FEMALE_PROSTITUTE_1_CHAT_1, 4u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return 28 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetWhiteFemaleProstituteTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FEMALE_PROSTITUTE_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FEMALE_PROSTITUTE_1_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FEMALE_PROSTITUTE_1_EVADE_1, 3u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_WHITE_FEMALE_PROSTITUTE_1_CAR_COLLISION_1, 4u);
		break;
	case SOUND_PED_SOLICIT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FEMALE_PROSTITUTE_1_SOLICIT_1, 8u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_FEMALE_PROSTITUTE_1_CHAT_1, 4u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return 25 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetBlackProjectFemaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_OLD_1_CAR_JACKED_1,
		          6u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_OLD_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_OLD_1_EVADE_1, 10u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_OLD_1_FLEE_RUN_1,
		          6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		cAudioManager::GetPhrase(
		    &sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_OLD_1_CAR_COLLISION_1,
		    7u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_OLD_1_CHAT_EVENT_1,
		          2u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_OLD_1_CHAT_1, 10u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackProjectFemaleYoungTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		cAudioManager::GetPhrase(
		    &sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_YOUNG_1_HANDS_COWER_1,
		    4u);
		break;
	case SOUND_PED_CAR_JACKED:
		sfx = AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_YOUNG_1_CAR_JACKED_1;
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_YOUNG_1_ROBBED_1,
		          2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_YOUNG_1_EVADE_1,
		          5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		cAudioManager::GetPhrase(
		    &sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_YOUNG_1_CAR_COLLISION_1,
		    6u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_YOUNG_1_CHAT_EVENT_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_PROJECT_FEMALE_YOUNG_1_CHAT_1, 7u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChinatownMaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_OLD_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_OLD_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_OLD_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_OLD_EVADE_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_OLD_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_OLD_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_OLD_CHAT_1, 7u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChinatownMaleYoungTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_YOUNG_HANDS_COWER_1, 2u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_YOUNG_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_YOUNG_ATTACK_1, 6u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_YOUNG_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_YOUNG_CAR_COLLISION_1,
		          6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_YOUNG_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_MALE_YOUNG_CHAT_1, 6u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChinatownFemaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_OLD_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_OLD_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_OLD_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_OLD_CAR_COLLISION_1,
		          5u);
		break;
	case SOUND_PED_CHAT_EVENT: sfx = AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_OLD_CHAT_EVENT_1; break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_OLD_CHAT_1, 6u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChinatownFemaleYoungTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_YOUNG_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_YOUNG_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_YOUNG_EVADE_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_YOUNG_CAR_COLLISION_1,
		          7u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_YOUNG_CHAT_EVENT_1, 4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHINATOWN_FEMALE_YOUNG_CHAT_1, 7u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetLittleItalyMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_MALE_1_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_MALE_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_MALE_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_MALE_1_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_MALE_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_MALE_1_CAR_COLLISION_1, 7u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_MALE_1_CHAT_1, 6u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return 30 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetLittleItalyFemaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_OLD_CAR_JACKED_1,
		          2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_OLD_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_OLD_EVADE_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_OLD_CAR_COLLISION_1,
		          7u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_OLD_CHAT_EVENT_1,
		          4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_OLD_CHAT_1, 7u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetLittleItalyFemaleYoungTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_YOUNG_CAR_JACKED_1,
		          2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_YOUNG_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_YOUNG_EVADE_1, 7u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_YOUNG_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_YOUNG_CHAT_EVENT_1,
		          4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_LITTLE_ITALY_FEMALE_YOUNG_CHAT_1, 6u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteDockerMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_DOCKER_MALE_HANDS_COWER_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_DOCKER_MALE_ATTACK_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_DOCKER_MALE_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_DOCKER_MALE_CAR_COLLISION_1, 4u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_DOCKER_MALE_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_DOCKER_MALE_CHAT_1, 5u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackDockerMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_DOCKER_MALE_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_DOCKER_MALE_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_DOCKER_MALE_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_DOCKER_MALE_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_DOCKER_MALE_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_DOCKER_MALE_CHAT_1, 5u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetScumMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_MALE_HANDS_COWER_1, 5u);
		break;
	case SOUND_PED_ROBBED: sfx = AUDIO_SAMPLE_PED_SCUM_MALE_ROBBED_1; break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_MALE_ATTACK_1, 0xAu);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_MALE_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_MALE_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_WAIT_DOUBLEBACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_MALE_WAIT_DOUBLEBACK_1, 3u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_MALE_CHAT_SEXY_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_MALE_CHAT_1, 9u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetScumFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_FEMALE_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_FEMALE_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_FEMALE_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_FEMALE_EVADE_1, 8u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_FEMALE_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SCUM_FEMALE_CHAT_1, 13u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteWorkerMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_WORKER_MALE_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_WORKER_MALE_ATTACK_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_WORKER_MALE_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_WORKER_MALE_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_WORKER_MALE_CHAT_SEXY_1, 2u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_WORKER_MALE_CHAT_1, 6u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackWorkerMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_WORKER_MALE_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_WORKER_MALE_ATTACK_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_WORKER_MALE_EVADE_1, 3u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_WORKER_MALE_CAR_COLLISION_1, 4u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_WORKER_MALE_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_WORKER_MALE_CHAT_1, 4u);

		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBusinessMaleYoungTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_EVADE_1, 4u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_FLEE_RUN_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_CAR_COLLISION_1,
		          6u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_YOUNG_1_CHAT_1, 6u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}

	if(model == 61) sfx += 32;
	return sfx;
}

uint32
cAudioManager::GetBusinessMaleOldTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_EVADE_1, 4u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_FLEE_RUN_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_MALE_OLD_1_CHAT_1, 5u);

		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteBusinessFemaleTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_EVADE_1, 6u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_FLEE_RUN_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_CHAT_EVENT_1, 4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BUSINESS_WOMAN_1_CHAT_1, 7u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}

	if(model == MI_B_WOM2) sfx += 34;
	return sfx;
}

uint32
cAudioManager::GetBlackBusinessFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_HANDS_COWER_1, 5u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_CAR_JACKED_1, 4u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_ROBBED_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_EVADE_1, 6u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_FLEE_RUN_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_CAR_COLLISION_1,
		          7u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_CHAT_EVENT_1, 4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_BUSINESS_FEMALE_CHAT_1, 7u);

		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetSupermodelMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_MALE_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_MALE_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_MALE_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_MALE_EVADE_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_MALE_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_MALE_CHAT_SEXY_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_MALE_CHAT_1, 6u);

		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetSupermodelFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_FEMALE_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_FEMALE_ROBBED_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_FEMALE_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_FEMALE_CAR_COLLISION_1, 7u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_FEMALE_CHAT_EVENT_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SUPERMODEL_FEMALE_CHAT_1, 8u);

		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetStewardMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_MALE_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_MALE_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_MALE_EVADE_1, 3u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_MALE_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_MALE_CHAT_1, 4u);

		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetStewardFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_FEMALE_1_HANDS_COWER_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_FEMALE_1_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_FEMALE_1_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STEWARD_FEMALE_1_CHAT_1, 5u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return 18 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetFanMaleTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_MALE_1_ATTACK_1, 3u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_MALE_1_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_MALE_1_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_MALE_1_CHAT_EVENT_1, 2u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_MALE_1_CHAT_1, 6u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}

	if(model == MI_FAN_MAN2) sfx += 20;
	return sfx;
}

uint32
cAudioManager::GetFanFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_ROBBED: sfx = AUDIO_SAMPLE_PED_FAN_FEMALE_1_ROBBED_1; break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_FEMALE_1_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_FEMALE_1_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_FEMALE_1_CHAT_EVENT_1, 2u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FAN_FEMALE_1_CHAT_1, 6u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return 18 * (m_sQueueSample.m_nEntityIndex & 1) + sfx;
}

uint32
cAudioManager::GetHospitalMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_MALE_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_MALE_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_MALE_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_MALE_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_MALE_CHAT_1, 5u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetHospitalFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_FEMALE_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_FEMALE_CAR_COLLISION_1, 6u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_HOSPITAL_FEMALE_CHAT_1, 6u);

		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetWhiteConstructionWorkerTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CONSTRUCTION_WORKER_HANDS_COWER_1,
		          3u);
		break;
	case SOUND_PED_CAR_JACKED:
		sfx = AUDIO_SAMPLE_PED_WHITE_CONSTRUCTION_WORKER_CAR_JACKED_1;
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CONSTRUCTION_WORKER_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CONSTRUCTION_WORKER_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_WHITE_CONSTRUCTION_WORKER_CAR_COLLISION_1, 4u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CONSTRUCTION_WORKER_CHAT_SEXY_1,
		          3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_WHITE_CONSTRUCTION_WORKER_CHAT_1, 7u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetBlackConstructionWorkerTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CONSTRUCTION_WORKER_HANDS_COWER_1,
		          3u);
		break;
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CONSTRUCTION_WORKER_CAR_JACKED_1,
		          2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CONSTRUCTION_WORKER_ATTACK_1, 5u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CONSTRUCTION_WORKER_EVADE_1, 5u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx,
		          AUDIO_SAMPLE_PED_BLACK_CONSTRUCTION_WORKER_CAR_COLLISION_1, 5u);
		break;
	case SOUND_PED_CHAT_SEXY:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CONSTRUCTION_WORKER_CHAT_SEXY_1,
		          4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BLACK_CONSTRUCTION_WORKER_CHAT_1, 4u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetShopperFemaleTalkSfx(int16 sound, int32 model)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_CAR_JACKED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SHOPPER_FEMALE_1_CAR_JACKED_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SHOPPER_FEMALE_1_ROBBED_1, 2u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SHOPPER_FEMALE_1_EVADE_1, 6u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SHOPPER_FEMALE_1_CAR_COLLISION_1, 7u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SHOPPER_FEMALE_1_CHAT_EVENT_1, 4u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SHOPPER_FEMALE_1_CHAT_1, 7u);
		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}

	if(model == MI_SHOPPER2) {
		sfx += 28;
	} else if(model == MI_SHOPPER3) {
		sfx += 56;
	}
	return sfx;
}

uint32
cAudioManager::GetStudentMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_MALE_HANDS_COWER_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_MALE_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_MALE_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_MALE_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_MALE_CAR_COLLISION_1, 4u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_MALE_CHAT_EVENT_1, 3u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_MALE_CHAT_1, 5u);

		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetStudentFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_COWER:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_FEMALE_HANDS_COWER_1, 4u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_FEMALE_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_FEMALE_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_FEMALE_EVADE_1, 4u);
		break;
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_FEMALE_CAR_COLLISION_1, 4u);
		break;
	case SOUND_PED_CHAT_EVENT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_FEMALE_CHAT_EVENT_1, 2u);
		break;
	case SOUND_PED_CHAT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_STUDENT_FEMALE_CHAT_1, 4u);

		break;
	default: return GetGenericFemaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetCasualMaleOldTalkSfx(int16 sound)
{
	return GetGenericMaleTalkSfx(sound);
}

uint32
cAudioManager::GetSpecialCharacterTalkSfx(int32 modelIndex, int32 sound)
{
	char *modelName = CModelInfo::GetModelInfo(modelIndex)->GetName();
	if(strcmp(modelName, "eight") == 0 || strcmp(modelName, "eight2") == 0) {
		return GetEightTalkSfx(sound);
	}
	if(strcmp(modelName, "frankie") == 0) { return GetFrankieTalkSfx(sound); }
	if(strcmp(modelName, "misty") == 0) { return GetMistyTalkSfx(sound); }
	if(strcmp(modelName, "ojg") == 0 || strcmp(modelName, "ojg_p") == 0) {
		return GetOJGTalkSfx(sound);
	}
	if(strcmp(modelName, "cat") == 0) { return GetCatatalinaTalkSfx(sound); }
	if(strcmp(modelName, "bomber") == 0) { return GetBomberTalkSfx(sound); }
	if(strcmp(modelName, "s_guard") == 0) { return GetSecurityGuardTalkSfx(sound); }
	if(strcmp(modelName, "chunky") == 0) { return GetChunkyTalkSfx(sound); }
	if(strcmp(modelName, "asuka") == 0) { return GetGenericFemaleTalkSfx(sound); }
	if(strcmp(modelName, "maria") == 0) { return GetGenericFemaleTalkSfx(sound); }

	return GetGenericMaleTalkSfx(sound);
}
uint32
cAudioManager::GetEightTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		cAudioManager::GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_EIGHT_HANDS_UP_1, 2u);
		break;
	case SOUND_PED_ROBBED:
		cAudioManager::GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_EIGHT_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		cAudioManager::GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_EIGHT_ATTACK_1, 6u);
		break;
	case SOUND_PED_EVADE:
		cAudioManager::GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_EIGHT_EVADE_1, 7u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetFrankieTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FRANKIE_HANDS_UP_1, 4u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FRANKIE_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FRANKIE_ATTACK_1, 6u);
		break;
	case SOUND_PED_EVADE:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FRANKIE_EVADE_1, 3u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetMistyTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MISTY_HANDS_UP_1, 5u);
		break;
	case SOUND_PED_ROBBED:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MISTY_ROBBED_1, 2u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MISTY_ATTACK_1, 4u);
		break;
	case SOUND_PED_EVADE: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MISTY_EVADE_1, 5u); break;
	case SOUND_PED_TAXI_CALL:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MISTY_THUMB_LIFT_1, 4u);
		break;
	default: return GetGenericFemaleTalkSfx(sound); break;
	}
	return sfx;
}

uint32
cAudioManager::GetOJGTalkSfx(int16 sound)
{
	return GetGenericMaleTalkSfx(sound);
}

uint32
cAudioManager::GetCatatalinaTalkSfx(int16 sound)
{
	return GetGenericFemaleTalkSfx(sound);
}

uint32
cAudioManager::GetBomberTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	if(sound != SOUND_PED_BOMBER) return GetGenericMaleTalkSfx(sound);

	GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_BOMBER_THREAT_1, 7u);
	return sfx;
}

uint32
cAudioManager::GetSecurityGuardTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_HANDS_UP:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SECURITY_GUARD_HANDS_UP_1, 2u);
		break;
	case SOUND_PED_HANDS_COWER: sfx = AUDIO_SAMPLE_PED_SECURITY_GUARD_HANDS_COWER_1; break;
	case SOUND_PED_CAR_JACKED:
	case SOUND_PED_CAR_COLLISION:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SECURITY_GUARD_CAR_EVENT_1, 6u);
		break;
	case SOUND_PED_ATTACK:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SECURITY_GUARD_ATTACK_1, 2u);
		break;
	case SOUND_PED_FLEE_RUN:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_SECURITY_GUARD_CAR_EVENT_1, 12u);
		break;
	default: return GetGenericMaleTalkSfx(sound);
	}
	return sfx;
}

uint32
cAudioManager::GetChunkyTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	if(sound == SOUND_PED_DEATH) return AUDIO_SAMPLE_PED_CHUNKY_DEATH_1;

	if(sound != SOUND_PED_FLEE_RUN) return GetGenericMaleTalkSfx(sound);

	GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_CHUNKY_FLEE_RUN_1, 5u);
	return sfx;
}

uint32
cAudioManager::GetGenericMaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_DEATH: GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_DEATH_1, 8u); break;
	case SOUND_PED_BULLET_HIT:
	case SOUND_PED_DEFEND:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_INJURED_PED_MALE_OUCH_1, 15u);
		break;
	case SOUND_PED_BURNING:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_BURNING_1, 8u);
		break;
	case SOUND_PED_FLEE_SPRINT:
		GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_MALE_FLEE_SPRINT_1, 6u);
		break;
	default: return NO_SAMPLE;
	}
	return sfx;
}

uint32
cAudioManager::GetGenericFemaleTalkSfx(int16 sound)
{
	uint32 sfx;
	static uint32 lastSfx = NO_SAMPLE;

	switch(sound) {
	case SOUND_PED_DEATH:
		cAudioManager::GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_DEATH_1, 10u);
		break;
	case SOUND_PED_BULLET_HIT:
	case SOUND_PED_DEFEND:
		cAudioManager::GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_INJURED_PED_FEMALE_OUCH_1,
		                         11u);
		break;
	case SOUND_PED_BURNING:
		cAudioManager::GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_BURNING_1, 9u);
		break;
	case SOUND_PED_FLEE_SPRINT:
		cAudioManager::GetPhrase(&sfx, &lastSfx, AUDIO_SAMPLE_PED_FEMALE_FLEE_SPRINT_1, 8u);
		break;
	default: return NO_SAMPLE;
	}
	return sfx;
}

WRAPPER void
cAudioManager::Service()
{
	EAXJMP(0x57A2A0);
}

STARTPATCHES
InjectHook(0x57B070, &cAudioManager::AddSampleToRequestedQueue, PATCH_JUMP);
InjectHook(0x57B210, &cAudioManager::AddDetailsToRequestedOrderList, PATCH_JUMP);
InjectHook(0x57B300, &cAudioManager::AddReflectionsToRequestedQueue, PATCH_JUMP);
InjectHook(0x57ABB0, &cAudioManager::ComputeVolume, PATCH_JUMP);

InjectHook(0x57A0E0, &cAudioManager::Initialise, PATCH_JUMP);
InjectHook(0x569420, &cAudioManager::PostInitialiseGameSpecificSetup, PATCH_JUMP);
//InjectHook(0x57EAC0, &cAudioManager::InitialisePoliceRadioZones, PATCH_JUMP);
//InjectHook(0x569650, &cAudioManager::ResetAudioLogicTimers, PATCH_JUMP);
InjectHook(0x57A150, &cAudioManager::Terminate, PATCH_JUMP);

InjectHook(0x57F050, &cAudioManager::GetMissionScriptPoliceAudioPlayingStatus, PATCH_JUMP);
InjectHook(0x5795D0, &cAudioManager::GetMissionAudioLoadingStatus, PATCH_JUMP);

InjectHook(0x57A8A0, &cAudioManager::GetNum3DProvidersAvailable, PATCH_JUMP);
InjectHook(0x57A8F0, &cAudioManager::AutoDetect3DProviders, PATCH_JUMP);
InjectHook(0x57A9C0, &cAudioManager::IsMP3RadioChannelAvailable, PATCH_JUMP);
InjectHook(0x57AA30, &cAudioManager::GetCDAudioDriveLetter, PATCH_JUMP);

InjectHook(0x57A730, &cAudioManager::SetEffectsMasterVolume, PATCH_JUMP);
InjectHook(0x57A750, &cAudioManager::SetMusicMasterVolume, PATCH_JUMP);
InjectHook(0x57A770, &cAudioManager::SetEffectsFadeVol, PATCH_JUMP);
InjectHook(0x57A790, &cAudioManager::SetMusicFadeVol, PATCH_JUMP);

InjectHook(0x57A9A0, &cAudioManager::SetSpeakerConfig, PATCH_JUMP);

InjectHook(0x56ECF0, &cAudioManager::ProcessJumboFlying, PATCH_JUMP);
InjectHook(0x569400, &cAudioManager::PreInitialiseGameSpecificSetup, PATCH_JUMP);

InjectHook(0x57F020, &cAudioManager::SetMissionScriptPoliceAudio, PATCH_JUMP);

InjectHook(0x5697A0, &cAudioManager::CalculateDistance, PATCH_JUMP);

InjectHook(0x56C3C0, &cAudioManager::UsesSiren, PATCH_JUMP);
InjectHook(0x56C3F0, &cAudioManager::UsesSirenSwitching, PATCH_JUMP);

InjectHook(0x579520, &cAudioManager::MissionScriptAudioUsesPoliceChannel, PATCH_JUMP);

InjectHook(0x57A8C0, &cAudioManager::Get3DProviderName, PATCH_JUMP);
InjectHook(0x56EA10, &cAudioManager::ProcessJumboTaxi, PATCH_JUMP);

InjectHook(0x579620, &cAudioManager::PlayLoadedMissionAudio, PATCH_JUMP);

InjectHook(0x5795F0, &cAudioManager::SetMissionAudioLocation, PATCH_JUMP);

InjectHook(0x57EFF0, &cAudioManager::ResetPoliceRadio, PATCH_JUMP);

InjectHook(0x57B030, &cAudioManager::InterrogateAudioEntities, PATCH_JUMP);

InjectHook(0x57C120, &cAudioManager::ClearRequestedQueue, PATCH_JUMP);
// InjectHook(0x580AF0, &cAudioManager::AgeCrimes, PATCH_JUMP);

InjectHook(0x56C600, &cAudioManager::UsesReverseWarning, PATCH_JUMP);
InjectHook(0x56CAB0, &cAudioManager::HasAirBrakes, PATCH_JUMP);

InjectHook(0x56F410, &cAudioManager::GetJumboTaxiFreq, PATCH_JUMP);

InjectHook(0x5699C0, &cAudioManager::ProcessPhysical, PATCH_JUMP);
// InjectHook(0x56E860, &cAudioManager::ProcessPlane, PATCH_JUMP);

InjectHook(0x5796A0, &cAudioManager::ClearMissionAudio, PATCH_JUMP);
// InjectHook(0x569700, &cAudioManager::ProcessReverb, PATCH_JUMP);

InjectHook(0x579650, &cAudioManager::IsMissionAudioSampleFinished, PATCH_JUMP);
// done
InjectHook(0x57AF90, &cAudioManager::RandomDisplacement, PATCH_JUMP);

InjectHook(0x57A9E0, &cAudioManager::ReleaseDigitalHandle, PATCH_JUMP);
InjectHook(0x57A9F0, &cAudioManager::RequireDigitalHandle, PATCH_JUMP);
InjectHook(0x57AA00, &cAudioManager::SetDynamicAcousticModelingStatus, PATCH_JUMP);

InjectHook(0x57AA50, &cAudioManager::IsAudioInitialised, PATCH_JUMP);

InjectHook(0x57A310, &cAudioManager::CreateEntity, PATCH_JUMP);
InjectHook(0x57A400, &cAudioManager::DestroyEntity, PATCH_JUMP);
InjectHook(0x57A4C0, &cAudioManager::SetEntityStatus, PATCH_JUMP);

InjectHook(0x569570, &cAudioManager::PreTerminateGameSpecificShutdown, PATCH_JUMP);
InjectHook(0x569640, &cAudioManager::PostTerminateGameSpecificShutdown, PATCH_JUMP);

InjectHook(0x57C290, &cAudioManager::GenerateIntegerRandomNumberTable, PATCH_JUMP);

InjectHook(0x56AD10, &cAudioManager::PlayerJustGotInCar, PATCH_JUMP);
InjectHook(0x56AD20, &cAudioManager::PlayerJustLeftCar, PATCH_JUMP);
InjectHook(0x570DB0, &cAudioManager::GetPhrase, PATCH_JUMP);

InjectHook(0x570E00, &cAudioManager::GetPlayerTalkSfx, PATCH_JUMP);
InjectHook(0x570EA0, &cAudioManager::GetCopTalkSfx, PATCH_JUMP);
InjectHook(0x570F80, &cAudioManager::GetSwatTalkSfx, PATCH_JUMP);
InjectHook(0x571040, &cAudioManager::GetFBITalkSfx, PATCH_JUMP);
InjectHook(0x571110, &cAudioManager::GetArmyTalkSfx, PATCH_JUMP);
InjectHook(0x5711C0, &cAudioManager::GetMedicTalkSfx, PATCH_JUMP);
InjectHook(0x5712B0, &cAudioManager::GetFiremanTalkSfx, PATCH_JUMP);
InjectHook(0x575340, &cAudioManager::GetNormalMaleTalkSfx, PATCH_JUMP);
InjectHook(0x575190, &cAudioManager::GetTaxiDriverTalkSfx, PATCH_JUMP);
InjectHook(0x575240, &cAudioManager::GetPimpTalkSfx, PATCH_JUMP);
InjectHook(0x571510, &cAudioManager::GetMafiaTalkSfx, PATCH_JUMP);
InjectHook(0x571650, &cAudioManager::GetTriadTalkSfx, PATCH_JUMP);
InjectHook(0x571770, &cAudioManager::GetDiabloTalkSfx, PATCH_JUMP);
InjectHook(0x5718D0, &cAudioManager::GetYakuzaTalkSfx, PATCH_JUMP);
InjectHook(0x5719E0, &cAudioManager::GetYardieTalkSfx, PATCH_JUMP);
InjectHook(0x571B00, &cAudioManager::GetColumbianTalkSfx, PATCH_JUMP);
InjectHook(0x571C30, &cAudioManager::GetHoodTalkSfx, PATCH_JUMP);
InjectHook(0x571D80, &cAudioManager::GetBlackCriminalTalkSfx, PATCH_JUMP);
InjectHook(0x571E60, &cAudioManager::GetWhiteCriminalTalkSfx, PATCH_JUMP);
InjectHook(0x571F40, &cAudioManager::GetMaleNo2TalkSfx, PATCH_JUMP);
InjectHook(0x572AF0, &cAudioManager::GetBlackProjectMaleTalkSfx, PATCH_JUMP);
InjectHook(0x5725D0, &cAudioManager::GetWhiteFatMaleTalkSfx, PATCH_JUMP);
InjectHook(0x5726C0, &cAudioManager::GetBlackFatMaleTalkSfx, PATCH_JUMP);
InjectHook(0x572050, &cAudioManager::GetBlackCasualFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x572170, &cAudioManager::GetWhiteCasualFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x572280, &cAudioManager::GetFemaleNo3TalkSfx, PATCH_JUMP);
InjectHook(0x5724D0, &cAudioManager::GetBlackFatFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x5727B0, &cAudioManager::GetWhiteFatFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x5728B0, &cAudioManager::GetBlackFemaleProstituteTalkSfx, PATCH_JUMP);
InjectHook(0x5729D0, &cAudioManager::GetWhiteFemaleProstituteTalkSfx, PATCH_JUMP);
InjectHook(0x572C20, &cAudioManager::GetBlackProjectFemaleOldTalkSfx, PATCH_JUMP);
InjectHook(0x572D20, &cAudioManager::GetBlackProjectFemaleYoungTalkSfx, PATCH_JUMP);
InjectHook(0x572E10, &cAudioManager::GetChinatownMaleOldTalkSfx, PATCH_JUMP);
InjectHook(0x572F10, &cAudioManager::GetChinatownMaleYoungTalkSfx, PATCH_JUMP);
InjectHook(0x573010, &cAudioManager::GetChinatownFemaleOldTalkSfx, PATCH_JUMP);
InjectHook(0x5730F0, &cAudioManager::GetChinatownFemaleYoungTalkSfx, PATCH_JUMP);
InjectHook(0x5731E0, &cAudioManager::GetLittleItalyMaleTalkSfx, PATCH_JUMP);
InjectHook(0x573310, &cAudioManager::GetLittleItalyFemaleOldTalkSfx, PATCH_JUMP);
InjectHook(0x573400, &cAudioManager::GetLittleItalyFemaleYoungTalkSfx, PATCH_JUMP);
InjectHook(0x5734F0, &cAudioManager::GetWhiteDockerMaleTalkSfx, PATCH_JUMP);
InjectHook(0x5735E0, &cAudioManager::GetBlackDockerMaleTalkSfx, PATCH_JUMP);
InjectHook(0x5736D0, &cAudioManager::GetScumMaleTalkSfx, PATCH_JUMP);
InjectHook(0x5737E0, &cAudioManager::GetScumFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x5738D0, &cAudioManager::GetWhiteWorkerMaleTalkSfx, PATCH_JUMP);
InjectHook(0x5739C0, &cAudioManager::GetBlackWorkerMaleTalkSfx, PATCH_JUMP);
InjectHook(0x5713E0, &cAudioManager::GetBusinessMaleYoungTalkSfx, PATCH_JUMP);
InjectHook(0x5712C0, &cAudioManager::GetBusinessMaleOldTalkSfx, PATCH_JUMP);
InjectHook(0x5723A0, &cAudioManager::GetWhiteBusinessFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x573AB0, &cAudioManager::GetBlackBusinessFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x573BD0, &cAudioManager::GetSupermodelMaleTalkSfx, PATCH_JUMP);
InjectHook(0x573CD0, &cAudioManager::GetSupermodelFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x573DC0, &cAudioManager::GetStewardMaleTalkSfx, PATCH_JUMP);
InjectHook(0x573E90, &cAudioManager::GetStewardFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x573F60, &cAudioManager::GetFanMaleTalkSfx, PATCH_JUMP);
InjectHook(0x574040, &cAudioManager::GetFanFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x574120, &cAudioManager::GetHospitalMaleTalkSfx, PATCH_JUMP);
InjectHook(0x5741F0, &cAudioManager::GetHospitalFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x574290, &cAudioManager::GetWhiteConstructionWorkerTalkSfx, PATCH_JUMP);
InjectHook(0x574380, &cAudioManager::GetBlackConstructionWorkerTalkSfx, PATCH_JUMP);
InjectHook(0x574480, &cAudioManager::GetShopperFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x574590, &cAudioManager::GetStudentMaleTalkSfx, PATCH_JUMP);
InjectHook(0x574690, &cAudioManager::GetStudentFemaleTalkSfx, PATCH_JUMP);
InjectHook(0x572040, &cAudioManager::GetCasualMaleOldTalkSfx, PATCH_JUMP);

InjectHook(0x574790, &cAudioManager::GetSpecialCharacterTalkSfx, PATCH_JUMP);
InjectHook(0x574DA0, &cAudioManager::GetEightTalkSfx, PATCH_JUMP);
InjectHook(0x574E50, &cAudioManager::GetFrankieTalkSfx, PATCH_JUMP);
InjectHook(0x574F00, &cAudioManager::GetMistyTalkSfx, PATCH_JUMP);
InjectHook(0x574FD0, &cAudioManager::GetOJGTalkSfx, PATCH_JUMP);
InjectHook(0x574FE0, &cAudioManager::GetCatatalinaTalkSfx, PATCH_JUMP);
InjectHook(0x574FF0, &cAudioManager::GetBomberTalkSfx, PATCH_JUMP);
InjectHook(0x575060, &cAudioManager::GetSecurityGuardTalkSfx, PATCH_JUMP);
InjectHook(0x575120, &cAudioManager::GetChunkyTalkSfx, PATCH_JUMP);

InjectHook(0x575460, &cAudioManager::GetGenericMaleTalkSfx, PATCH_JUMP);
InjectHook(0x575510, &cAudioManager::GetGenericFemaleTalkSfx, PATCH_JUMP);
ENDPATCHES