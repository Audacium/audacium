#!/bin/sh
#
# autogen.sh - Generates initial makefiles from a pristine source tree
#
# USAGE:
#   autogen.sh [configure options]
#
# Configuration is affected by environment variables as follows:
#
# DRYRUN
#  If set to any value it will do no configuring but  will emit the
#  programs that would be run.
#   e.g. DRYRUN=1 ./autogen.sh
#
# NOCONFIGURE
#  If set to any value it will generate all files but not invoke the
#  generated configure script.
#   e.g. NOCONFIGURE=1 ./autogen.sh
#
# AUTOMAKE ACLOCAL AUTOCONF AUTOHEADER LIBTOOLIZE GTKDOCIZE
#  If set (named after program) then this overrides any searching for
#  the programs on the current PATH.
#   e.g. AUTOMAKE=automake-1.7 ACLOCAL=aclocal-1.7 ./autogen.sh
#
# CONFIG_DIR (default ../config)
#  The directory where fresh GNU config.guess and config.sub can be
#  found for automatic copying in-place.
#
# PATH
#  Where the programs are searched for
#
# SRCDIR (default .)
#  Source directory
#
# This script is based on similar scripts used in various tools
# commonly made available via CVS and used with GNU automake.
# Try 'locate autogen.sh' on your system and see what you get.
#
# This script is in the public domain
#

# Directory for the sources
SRCDIR=${SRCDIR-.}

# Where the GNU config.sub, config.guess might be found
CONFIG_DIR=${CONFIG_DIR-../config}

# GIT sub modules file
GITMODULES='.gitmodules'

# The programs required for configuring which will be searched for
# in the current PATH.
# Set an envariable of the same name in uppercase, to override scan
#
programs="automake aclocal autoconf autoheader libtoolize"
confs=`find . -name configure.ac -print | grep -v /releases/`

gtkdoc_args=
if grep "^GTK_DOC_CHECK" $confs >/dev/null; then
  programs="$programs gtkdocize"
  gtkdoc_args="--enable-gtk-doc"
fi
if grep "^AC_CHECK_PROGS.SWIG" $confs >/dev/null; then
  programs="$programs swig"
fi
ltdl_args=
if grep "^AC_LIBLTDL_" $confs >/dev/null; then
  ltdl_args="--ltdl"
fi
silent_args=
if grep "^AM_SILENT_RULES" $confs >/dev/null; then
  silent_args="--enable-silent-rules"
fi

# Some dependencies for autotools:
# automake 1.13 requires autoconf 2.65
# automake 1.12 requires autoconf 2.62
# automake 1.11 requires autoconf 2.62 (needed for AM_SILENT_RULES)
automake_min_vers=011102
aclocal_min_vers=$automake_min_vers
autoconf_min_vers=026200
autoheader_min_vers=$autoconf_min_vers
# libtool 2.2 required for LT_INIT language fix
libtoolize_min_vers=020200
gtkdocize_min_vers=010300
swig_min_vers=010324

# Default program arguments
automake_args="--gnu --add-missing --force --copy -Wall"
aclocal_args="-Wall"
autoconf_args="-Wall"
libtoolize_args="--force --copy --automake $ltdl_args"
gtkdocize_args="--copy"
# --enable-gtk-doc does no harm if it's not available
configure_args="--enable-maintainer-mode $gtkdoc_args $silent_args"


# You should not need to edit below here
######################################################################


# number comparisons may need a C locale
LANG=C
LC_NUMERIC=C


program=`basename $0`

if test "X$DRYRUN" != X; then
  DRYRUN=echo
fi

cat > autogen-get-version.pl <<EOF
use File::Basename;
my \$prog=basename \$0;
die "\$prog: USAGE PATH PROGRAM-NAME\n  e.g. \$prog /usr/bin/foo-123 foo\n"
  unless @ARGV==2;

my(\$path,\$name)=@ARGV;
exit 0 if !-f \$path;
die "\$prog: \$path not found\n" if !-r \$path;

# Remove leading g and make it optional for glibtoolize etc.
my \$mname=\$name; \$mname =~ s/^g//;

my(@vnums);
for my \$varg (qw(--version -version)) {
  my \$cmd="\$path \$varg";
  open(PIPE, "\$cmd 2>&1 |") || next;
  while(<PIPE>) {
    chomp;
    next if @vnums; # drain pipe if we got a vnums
    # Add optional leading g
    next unless /^g?\$mname/i;
    my(\$v)=/(\S+)\$/i; \$v =~ s/-.*\$//;
    @vnums=grep { defined \$_ && !/^\s*\$/} map { s/\D//g; \$_; } split(/\./, \$v);
  }
  close(PIPE);
  last if @vnums;
}

@vnums=(@vnums, 0, 0, 0)[0..2];
\$vn=join('', map { sprintf('%02d', \$_) } @vnums);
print "\$vn\n";
exit 0;
EOF

autogen_get_version="`pwd`/autogen-get-version.pl"

trap "rm -f $autogen_get_version" 0 1 9 15


update_prog_version() {
  dir=$1
  prog=$2

  # If there exists an envariable PROG in uppercase, use that and do not scan
  ucprog=`echo $prog | tr 'a-z' 'A-Z' `
  eval env=\$${ucprog}
  if test X$env != X; then
    prog_name=$env
    prog_vers=`perl $autogen_get_version $prog_name $prog`

    if test X$prog_vers = X; then
      prog_vers=0
    fi
    eval ${prog}_name=${prog_name}
    eval ${prog}_vers=${prog_vers}
    eval ${prog}_dir=environment
    return
  fi

  eval prog_name=\$${prog}_name
  eval prog_vers=\$${prog}_vers
  eval prog_dir=\$${prog}_dir
  if test X$prog_vers = X; then
    prog_vers=0
  fi

  save_PATH="$PATH"

  cd "$dir"
  PATH=".:$PATH"

  nameglob="$prog*"
  if [ -x /usr/bin/uname ]; then
    if [ `/usr/bin/uname` = 'Darwin' -a $prog = 'libtoolize' ] ; then
      nameglob="g$nameglob"
    fi
  fi
  names=`ls $nameglob 2>/dev/null`
  if [ "X$names" != "X" ]; then
    for name in $names; do
      vers=`perl $autogen_get_version $dir/$name $prog`
      if [ "X$vers" = "X" ]; then
        continue
      fi

      if expr $vers '>' $prog_vers >/dev/null; then
        prog_name=$name
        prog_vers=$vers
        prog_dir="$dir"
      fi
    done
  fi

  eval ${prog}_name=${prog_name}
  eval ${prog}_vers=${prog_vers}
  eval ${prog}_dir=${prog_dir}

  PATH="$save_PATH"
}


check_prog_version() {
  prog=$1

  eval min=\$${prog}_min_vers

  eval prog_name=\$${prog}_name
  eval prog_vers=\$${prog}_vers
  eval prog_dir=\$${prog}_dir

  echo "$program: $prog program '$prog_name' V $prog_vers (min $min) in $prog_dir" 1>&2

  rc=1
  if test $prog_vers != 0; then
    if expr $prog_vers '<' $min >/dev/null; then
       echo "$program: ERROR: \`$prog' version $prog_vers in $prog_dir is too old."
       echo "    (version $min or newer is required)"
       rc=0
    else
      # Things are ok, so set the ${prog} name
      eval ${prog}=${prog_name}
    fi
  else
    echo "$program: ERROR: You must have \`$prog' installed to compile this package."
    echo "     (version $min or newer is required)"
    rc=0
  fi

  return $rc
}


# Find newest version of programs in the current PATH
save_args=${1+"$*"}
save_ifs="$IFS"
IFS=":"
set - $PATH
IFS="$save_ifs"

echo "$program: Looking for programs: $programs"

here=`pwd`
while [ $# -ne 0 ] ; do
  dir=$1
  shift
  if [ ! -d "$dir" ]; then
    continue
  fi

  for prog in $programs; do
    update_prog_version "$dir" $prog
  done
done
cd $here

set - $save_args
# END Find programs


# Check the versions meet the requirements
for prog in $programs; do
  if check_prog_version $prog; then
    exit 1
  fi
done

echo "$program: Dependencies satisfied"

if test -d $SRCDIR/libltdl; then
  touch $SRCDIR/libltdl/NO-AUTO-GEN
fi

config_dir=
if test -d $CONFIG_DIR; then
  config_dir=`cd $CONFIG_DIR; pwd`
fi


# Initialise and/or update GIT submodules
if test -f $GITMODULES ; then
  echo " "
  modules=`sed -n -e 's/^.*path = \(.*\)/\1/p' $GITMODULES`
  for module in $modules; do
    if test `ls -1 $module | wc -l` -eq 0; then
       echo "$program: Initializing git submodule in $module"
       $DRYRUN git submodule init $module
    fi
  done
  echo "$program: Updating git submodules: $modules"
  $DRYRUN git submodule update
fi

for coin in `find $SRCDIR -name configure.ac -print | grep -v /releases/`
do
  status=0
  dir=`dirname $coin`
  if test -f "$dir/NO-AUTO-GEN"; then
    echo "$program: Skipping $dir -- flagged as no auto-generation"
  else
    echo " "
    echo $program: Processing directory $dir
    ( cd "$dir"

      # Ensure that these are created by the versions on this system
      # (indirectly via automake)
      $DRYRUN rm -f ltconfig ltmain.sh libtool stamp-h*
      # Made by automake
      $DRYRUN rm -f missing depcomp
      # automake junk
      $DRYRUN rm -rf autom4te*.cache

      config_macro_dir=`sed -ne 's/^AC_CONFIG_MACRO_DIR(\([^)]*\).*/\1/p' configure.ac`
      if test "X$config_macro_dir" = X; then
	config_macro_dir=.
      else
        aclocal_args="$aclocal_args -I $config_macro_dir "
      fi

      config_aux_dir=`sed -ne 's/^AC_CONFIG_AUX_DIR(\([^)]*\).*/\1/p' configure.ac`
      if test "X$config_aux_dir" = X; then
	config_aux_dir=.
      fi

      if test "X$config_dir" != X; then
        echo "$program: Updating config.guess and config.sub"
	for file in config.guess config.sub; do
	  cfile=$config_dir/$file
          xfile=$config_aux_dir/$file
	  if test -f $cfile; then
	    $DRYRUN rm -f $xfile
	    $DRYRUN cp -p $cfile $xfile
	  fi
	done
      fi

      echo "$program: Running $libtoolize $libtoolize_args"
      $DRYRUN rm -f ltmain.sh libtool
      eval $DRYRUN $libtoolize $libtoolize_args
      status=$?
      if test $status != 0; then
	  break
      fi

      if grep "^GTK_DOC_CHECK" configure.ac >/dev/null; then
        # gtkdocize junk
        $DRYRUN rm -rf gtk-doc.make
        echo "$program: Running $gtkdocize $gtkdocize_args"
        $DRYRUN $gtkdocize $gtkdocize_args
        status=$?
	if test $status != 0; then
	    break
	fi
      fi

      echo "$program: Running $aclocal $aclocal_args"
      $DRYRUN $aclocal $aclocal_args
      if grep "^A[CM]_CONFIG_HEADER" configure.ac >/dev/null; then
	echo "$program: Running $autoheader"
	$DRYRUN $autoheader
        status=$?
	if test $status != 0; then
	    break
	fi
      fi
      echo "$program: Running $automake $automake_args"
      $DRYRUN $automake $automake_args
      status=$?
      if test $status != 0; then
	  break
      fi

      echo "$program: Running $autoconf $autoconf_args"
      $DRYRUN $autoconf $autoconf_args
      status=$?
      if test $status != 0; then
	  break
      fi
    )
  fi

  if test $status != 0; then
    echo "$program: FAILED to configure $dir"
    exit $status
  fi

done



rm -f config.cache

AUTOMAKE=$automake
AUTOCONF=$autoconf
ACLOCAL=$aclocal
export AUTOMAKE AUTOCONF ACLOCAL

if test "X$NOCONFIGURE" = X; then
  echo " "
  if test -z "$*"; then
    echo "$program: WARNING: Running \`configure' with no arguments."
    echo "If you wish to pass any to it, please specify them on the"
    echo "\`$program' command line."
  fi

  echo "$program: Running ./configure $configure_args $@"
  if test "X$DRYRUN" = X; then
    $DRYRUN ./configure $configure_args "$@" \
    && echo "$program: Now type \`make' to compile this package" || exit 1
  else
    $DRYRUN ./configure $configure_args "$@"
  fi
fi

exit $status
