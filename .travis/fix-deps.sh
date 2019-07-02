
# Macdeployqt don't like symlink, it uses install id to reference Frameworks (and dylib)
# When a library is found with a path as install id and referenced with a symlink to this path as link inside a lib, it fails
# I'm not sure but there might be a version related issue (eg. libavcodec.58.dylib vs libavocdec.58.X.X.dylib)
# https://bugreports.qt.io/browse/QTBUG-56814

set -x

formula_to_fix=(
    ffmpeg
    nettle
)

for formula in "${formula_to_fix[@]}"; do
    cellar_path=$(brew --cellar $formula)
    full_path=$(brew info $formula | grep "^$cellar_path.*\*$" | cut -d' ' -f1)
    lib_path=$full_path'/lib'
    libs_to_fix=$(find $lib_path -type f -maxdepth 1 | grep "^.*\.dylib$")
    
    cellar_sed=$(echo $full_path | sed -e 's/[]\/$*.^[]/\\&/g')
    opt_sed=$(brew --prefix $formula | sed -e 's/[]\/$*.^[]/\\&/g')

    OIFS=$IFS
    IFS=$'\n'
    libs_to_fix=$libs_to_fix

    for lib in $libs_to_fix
    do

        otool -L $lib | grep "[[:space:]]$full_path" | while read -r line ; do
            install_path="$(cut -d' ' -f1 <<<"$line")"
            install_path="$(echo $install_path | tr -d '[:space:]')"
            new_install_path="$(echo $install_path | sed -e "s/$cellar_sed/$opt_sed/")"
            sudo install_name_tool -change "$install_path" "$new_install_path" "$lib"
        done

    done

done
