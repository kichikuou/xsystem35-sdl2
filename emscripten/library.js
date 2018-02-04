mergeInto(LibraryManager.library, {
	wait_vsync: function() {
		EmterpreterAsync.handle(function(resume) {
			window.requestAnimationFrame(function() {
				if (ABORT) return; // do this manually; we can't call into Browser.safeSetTimeout, because that is paused/resumed!
				resume();
			});
		});
	}
});
