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
open(FS,"</usr/include/SDL/SDL_keysym.h") || exit;
while(<FS>) {
    if( /SDLK_([^\s]+)\s+=\s*([^,]*),/ ) {
	$sdlk_list[eval($2)]="$1";
    }
}
close(FS);


print "static int sdl_keytable[SDLK_LAST] = {\n\t";
for( $i = 0 ; $i < scalar(@sdlk_list) ; $i ++ ) {
    #print "$i @sdlk_list[$i] $keyh_list{@sdlk_list[$i]}\n";
    $x = @sdlk_list[$i];
    if( length($x) == 0 ) {
	print "KEY_UNDEFINED, ";
    } elsif ( $keyh_list{$x} ) {
	print "KEY_@key_list[$keyh_list{$x}], ";
    } elsif ( $x =~ /[a-z]/ ) {
	print "KEY_".uc($x).", ";
    } elsif ( $x =~ /KP/ ) {
	if( $x =~ /[0-9]/ ) {
	    $x =~ s/^KP/PAD_/;
	} else {
	    $x =~ s/^KP/PAD/;
	}
	print "KEY_$x, ";
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
