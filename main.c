// by mothcompute
// too lazy to include the unlicense but just pretend i did. thats the license
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <switch.h>
#include <unistd.h>
#include <dirent.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <xmp.h>

int playing = 0, files = HidNpadButton_Minus, p = 0;

void pmod(void *udata, uint8_t *stream, int len) {
	if(playing && p) playing = xmp_play_buffer((xmp_context)udata, stream, len, 0) >= 0;
	else memset(stream, 0, len);
}

int main(int argc, char** argv) {
	consoleInit(NULL);
	SDL_Init(SDL_INIT_AUDIO);
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	PadState pad;
	padInitializeDefault(&pad);

	consoleUpdate(NULL);
	
	xmp_context c = xmp_create_context();
	struct xmp_module_info mi;
	struct xmp_frame_info fi;

	SDL_AudioSpec as;
	as.freq = 48000;
	as.format = AUDIO_S16;
	as.channels = 2;
	as.samples = 480;
	as.callback = pmod;
	as.userdata = c;
	if(SDL_OpenAudio(&as, NULL)) return 1;


	// calculated the number 50 from the avg. number of files in each directory on my computer (51)
	// rounded down. subsequent resizes only add 16 to avoid reallocating too much memory
	DIR* d;
	struct dirent* dir = malloc(50*sizeof(struct dirent)), *tmpdir;
	long num_dir = 0, sz_dir = 50, frame = 0, pos = 0;

	chdir("/");

	while (appletMainLoop()) {
		printf("\e[1;1H");
		padUpdate(&pad);
		u64 kDown = padGetButtonsDown(&pad);
		if (kDown & HidNpadButton_Plus) break;
		int mchg = kDown & HidNpadButton_Minus;
		files ^= mchg;
		if(mchg) {
			printf("\e[2J");
			if(files) {
				num_dir = 0;
				d = opendir(".");
				if(d) {
					while((tmpdir = readdir(d)) != NULL) {
						dir[num_dir++] = *tmpdir;
						if(num_dir >= sz_dir-1) dir = realloc(dir, (sz_dir += 16)*sizeof(struct dirent));
					}
					closedir(d);	
				}
			}
		}
		if(files) {
			if((kDown & HidNpadButton_Down)) {
				if(frame*44 + pos < num_dir - 1) pos++;
				if(pos == 44) {
					printf("\e[2J");
					frame++;
					pos = 0;
				}
			}
			if((kDown & HidNpadButton_Up)) {
				if(frame || pos > 0) pos--;
				if(pos == -1) {
					printf("\e[2J");
					if(frame) frame--;
					pos = 43;
				}
			}
			if(kDown & HidNpadButton_B) {
				printf("\e[2J");
				num_dir = 0;
				chdir("..");
				d = opendir(".");
				if(d) {
					while((tmpdir = readdir(d)) != NULL) {
						dir[num_dir++] = *tmpdir;
						if(num_dir >= sz_dir-1) dir = realloc(dir, (sz_dir += 16)*sizeof(struct dirent));
					}
					closedir(d);
					pos = 0;
					frame = 0;
				}
			}
			if(kDown & HidNpadButton_A) {
				struct stat s;
				stat(dir[pos + frame*44].d_name, &s);
				printf("\e[2J");
				if(S_ISDIR(s.st_mode)) {
					num_dir = 0;
					chdir(dir[pos + frame*44].d_name);
					d = opendir(".");
					if(d) {
						while ((tmpdir = readdir(d)) != NULL) {
							dir[num_dir++] = *tmpdir;
							if(num_dir >= sz_dir-1) dir = realloc(dir, (sz_dir += 16)*sizeof(struct dirent));
						}
						closedir(d);
						pos = 0;
						frame = 0;
					}
				} else if(S_ISREG(s.st_mode)) {
					struct xmp_test_info ti;
					if(!xmp_test_module(dir[pos + frame*44].d_name, &ti)) {
						if(playing) {
							xmp_end_player(c);
							xmp_release_module(c);
						}
						xmp_load_module(c, dir[pos + frame*44].d_name);
						xmp_get_module_info(c, &mi);
						if((playing = !xmp_start_player(c, 48000, 0))) SDL_PauseAudio(0);
						p = HidNpadButton_A;
					}
				}
			}
			long s = num_dir-(44*frame) >= 44 ? 44*(frame+1) : num_dir;
			for(long i = 44*frame; i < s; i++) {
				//printf("\e[%im%li\n", i % 44 == pos, s);
				//printf("\e[%im%.79s\n", i % 44 == pos, dir[i + frame*44].d_name);
				printf("\e[%im%.79s\n", i % 44 == pos, dir[i].d_name);
				//printf("\e[%im%li/%li %li %li\n", i % 44 == pos, i, num_dir-1, frame, pos);
			}
		} else {
			if(playing) {
				xmp_get_frame_info(c, &fi);
				printf("%s\n%02X.%02X %02X/%02X\n%li %li %li %li", mi.mod->name, fi.pos, mi.mod->len-1, fi.row, fi.num_rows, sz_dir, num_dir, frame, pos);
			}
			p ^= kDown & HidNpadButton_A;
		}
		
		SDL_Delay(5);

		consoleUpdate(NULL);
	}

	SDL_CloseAudio();
	SDL_Quit();
	consoleExit(NULL);
	return 0;
}
