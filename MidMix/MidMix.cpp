// MidMix.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <string>
#include <fstream>
#include <vector>
#include <climits>
#include <thread>

void writeLE(std::ofstream &outFile, size_t size, unsigned value)
{
	outFile.write(reinterpret_cast<const char*>(&value), size);
}

void createWavFromRaw(const std::string &rawPath, const std::string &wavPath)
{
	int channelCount = 2;
	int sampleSize = 2;
	int sampleRate = 44100;
	int blockAlign = sampleSize * channelCount;
	std::ifstream inFile(rawPath, std::ios::binary | std::ios::ate);
	int inSize = (int)inFile.tellg();
	inFile.seekg(0);
	std::vector<short> rawBuffer(inSize / sizeof(short));
	inFile.read((char*)&rawBuffer[0], inSize);
	inFile.close();

	//Normalize
	short peak = 0;
	for (short sample : rawBuffer)
	{
		if (peak < abs(sample))
			peak = abs(sample);
	}
	float normalizingScale = SHRT_MAX * 0.89125f / peak; //Leave 1 dB headroom for lossy compression
	for (short &sample : rawBuffer)
		sample = (short)(sample * normalizingScale);

	
	std::ofstream outFile(wavPath, std::ios::binary | std::ios::out);
	outFile << "RIFF";
	writeLE(outFile, 4, 36 + inSize);
	outFile << "WAVEfmt ";
	writeLE(outFile, 4, 16);
	writeLE(outFile, 2, 1);
	writeLE(outFile, 2, channelCount);
	writeLE(outFile, 4, sampleRate);
	writeLE(outFile, 4, sampleRate * blockAlign);
	writeLE(outFile, 2, blockAlign);
	writeLE(outFile, 2, sampleSize * 8);
	outFile << "data";
	writeLE(outFile, 4, inSize);
	outFile.write((const char*)&rawBuffer[0], inSize);
	outFile.close();
}

fluid_settings_t* settings;
fluid_synth_t* synth;;
int sfId;
fluid_player_t* player;
fluid_file_renderer_t* renderer;

extern "C" __declspec(dllexport)
void init()
{
	//Create settings
	settings = new_fluid_settings();
	// use number of samples processed as timing source, rather than the system timer
	fluid_settings_setstr(settings, "player.timing-source", "sample");
	//Since this is a non-realtime scenario, there is no need to pin the sample data
	fluid_settings_setint(settings, "synth.lock-memory", 0);
	
	//Use multiple cores
	auto coreCount = std::thread::hardware_concurrency();
	fluid_settings_setint(settings, "synth.cpu-cores", coreCount == 0 ? 2 : coreCount); //If unable to detect core count, assume at least two
	
	//Create synth
	synth = new_fluid_synth(settings);
	//Set the audio driver setting to file output
	//fluid_settings_setstr(settings, "audio.driver", "file");
	//Load sound font
	sfId = fluid_synth_sfload(synth, "soundfont.sf2", true);	
}

extern "C" __declspec(dllexport)
BOOL sfLoaded()
{
	return sfId > -1;
}

extern "C" __declspec(dllexport)
void mixdown(char *midiPath, char *mixdownPath)
{
	//Set raw mixdown path
	char rawMixdownPath[256];
	strcpy_s(rawMixdownPath, mixdownPath);
	size_t mixdownPathLen = strlen(mixdownPath);
	rawMixdownPath[mixdownPathLen++] = '_';
	rawMixdownPath[mixdownPathLen] = 0;
	
	fluid_settings_setstr(settings, "audio.file.name", rawMixdownPath);
	//fluid_settings_setstr(settings, "audio.file.name", "test.raw");
	
	//Create midi player
	player = new_fluid_player(synth);

	//Load midi file and start playing
	fluid_player_add(player, midiPath);
	fluid_player_play(player);
	///Create audio driver
	///fluid_audio_driver_t* adriver = new_fluid_audio_driver(settings, synth);
	
	//Create renderer
	renderer = new_fluid_file_renderer(synth);

	//Render to file
	while (fluid_player_get_status(player) == FLUID_PLAYER_PLAYING)
	{
		if (fluid_file_renderer_process_block(renderer) != FLUID_OK)
		{
			break;
		}
	}
	// just for sure: stop the playback explicitly and wait until finished
	fluid_player_stop(player);
	fluid_player_join(player);

	delete_fluid_file_renderer(renderer);
	delete_fluid_player(player);
	createWavFromRaw(rawMixdownPath, mixdownPath);
}

extern "C" __declspec(dllexport)
void close()
{
	delete_fluid_synth(synth);
	delete_fluid_settings(settings);
}




