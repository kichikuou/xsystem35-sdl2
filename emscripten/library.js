mergeInto(LibraryManager.library, {
	wait_vsync: function() {
		Asyncify.handleSleep(function(wakeUp) {
			window.requestAnimationFrame(function() {
				wakeUp();
			});
		});
	},
	load_mincho_font: function() {
		return Asyncify.handleSleep(function(wakeUp) {
			xsystem35.load_mincho_font().then(wakeUp);
		});
	},
	muspcm_init: function(audio_buffer_size) {
		return 0; // OK
	},
	muspcm_exit: function() {
		return 0; // OK
	},
	muspcm_load_no: function(slot, no) {  // async
		return xsystem35.audio.pcm_load(slot, no);
	},
	muspcm_load_mixlr: function(slot, noL, noR) {  // async
		return xsystem35.audio.pcm_load_mixlr(slot, noL, noR);
	},
	muspcm_unload: function(slot) {
		return xsystem35.audio.pcm_unload(slot);
	},
	muspcm_start: function(slot, loop) {
		return xsystem35.audio.pcm_start(slot, loop);
	},
	muspcm_stop: function(slot) {
		return xsystem35.audio.pcm_stop(slot);
	},
	muspcm_fadeout: function(slot, msec) {
		return xsystem35.audio.pcm_fadeout(slot, msec);
	},
	muspcm_pause: function(slot) {
		console.log('muspcm_pause: not implemented');
		return -1; // NG
	},
	muspcm_unpause: function(slot) {
		console.log('muspcm_unpause: not implemented');
		return -1; // NG
	},
	muspcm_getpos: function(slot) {
		return xsystem35.audio.pcm_getpos(slot);
	},
	muspcm_setvol: function(dev, slot, lv) {
		return xsystem35.audio.pcm_setvol(slot, lv);
	},
	muspcm_getwavelen: function(slot) {
		var len = xsystem35.audio.pcm_getwavelen(slot);
		return len > 65535 ? 65535 : len;
	},
	muspcm_isplaying: function(slot) {
		return xsystem35.audio.pcm_isplaying(slot);
	},
	muspcm_waitend: function(slot) {  // async
		return xsystem35.audio.pcm_waitend(slot);
	}
});
