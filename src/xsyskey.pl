#! /usr/bin/perl

@key_list=();
%keyh_list=();
open(FX,"<key.h") || exit;
while(<FX>) {
    if( /^#define KEY_([^\s]*)\s+[(]([^)]*)[)]/ ) {
	$key_list[eval($2)]="$1";
	$keyh_list{$1}=eval($2);
   }
}
close(fp);

@sdlk_list=();
open(FS,"<$ARGV[0]/SDL_scancode.h") || exit;
while(<FS>) {
    if( /SDL_SCANCODE_([^\s]+)\s+=\s*([^,]*),/ ) {
	$sdlk_list[eval($2)]="$1";
    }
}
close(FS);


print "static int sdl_keytable[SDL_NUM_SCANCODES] = {\n\t";
for( $i = 0 ; $i < scalar(@sdlk_list) ; $i ++ ) {
    #print "$i @sdlk_list[$i] $keyh_list{@sdlk_list[$i]}\n";
    $x = @sdlk_list[$i];
    if( length($x) == 0 ) {
	print "KEY_UNDEFINED, ";
    } elsif ( $keyh_list{$x} ) {
	print "KEY_@key_list[$keyh_list{$x}], ";
    } elsif ( $x =~ /[a-z]/ ) {
	print "KEY_".uc($x).", ";
    } elsif ( $x =~ /KP_/ ) {
	$x =~ s/^KP_/PAD_/;
	if ( $keyh_list{$x} ) {
	    print "KEY_$x, ";
	} else {
	    print "0 /* SDL_$x */, ";
	}
    } elsif ( $i < 0x61) {
	print "$i, ";
    } else {
	print "0 /* SDL_$x */, ";
    }
    if(($i % 4) == 3 ) {
	print "\n\t";
    }
}
print "};\n";
