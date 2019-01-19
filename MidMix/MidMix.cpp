// MidMix.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

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
	//Create synth
	synth = new_fluid_synth(settings);
	//Set the audio driver setting to file output
	fluid_settings_setstr(settings, "audio.driver", "file");
	//Load sound font
	sfId = fluid_synth_sfload(synth, "OmegaGMGS2.sf2", true);
	// use number of samples processed as timing source, rather than the system timer
	fluid_settings_setstr(settings, "player.timing-source", "sample");
	//Since this is a non-realtime szenario, there is no need to pin the sample data
	fluid_settings_setint(settings, "synth.lock-memory", 0);
	//Create midi player
	player = new_fluid_player(synth);
	//Create renderer
	fluid_file_renderer_t* renderer;
}

extern "C" __declspec(dllexport)
BOOL sfLoaded()
{
	return sfId > -1;
}

extern "C" __declspec(dllexport)
void mixdown(char *midiPath, char *mixdownPath)
{
	//Set mixdown path
	fluid_settings_setstr(settings, "audio.file.name", mixdownPath);
	//Load midi file and start playing
	fluid_player_add(player, midiPath);
	fluid_player_play(player);
	//Create audio driver
	//fluid_audio_driver_t* adriver = new_fluid_audio_driver(settings, synth);
	
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

}

extern "C" __declspec(dllexport)
void close()
{
	delete_fluid_file_renderer(renderer);
	delete_fluid_player(player);
	delete_fluid_synth(synth);
	delete_fluid_settings(settings);
}


