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
});
