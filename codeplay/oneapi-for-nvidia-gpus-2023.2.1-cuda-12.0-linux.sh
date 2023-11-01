#!/bin/sh
# shellcheck shell=sh

# Copyright (C) Codeplay Software Limited. All rights reserved.

checkArgument() {
  firstChar=$(echo "$1" | cut -c1-1)
  if [ "$firstChar" = '' ] || [ "$firstChar" = '-' ]; then
    printHelpAndExit
  fi
}

checkCmd() {
  if ! "$@"; then
    echo "Error - command failed: $*"
    exit 1
  fi
}

extractPackage() {
  fullScriptPath=$(readlink -f "$0")
  archiveStart=$(awk '/^__ARCHIVE__/ {print NR + 1; exit 0; }' "$fullScriptPath")

  checksum=$(tail "-n+$archiveStart" "$fullScriptPath" | sha384sum | awk '{ print $1 }')
  if [ "$checksum" != "$archiveChecksum" ]; then
    echo "Error: archive corrupted!"
    echo "Expected checksum: $archiveChecksum"
    echo "Actual checksum: $checksum"
    echo "Please try downloading this installer again."
    echo
    exit 1
  fi

  if [ "$tempDir" = '' ]; then
    tempDir=$(mktemp -d /tmp/oneapi_installer.XXXXXX)
  else
    checkCmd 'mkdir' '-p' "$tempDir"
    tempDir=$(readlink -f "$tempDir")
  fi

  tail "-n+$archiveStart" "$fullScriptPath" | tar -xz -C "$tempDir"
}

findOneapiRootOrExit() {
  for path in "$@"; do
    if [ "$path" != '' ] && [ -d "$path/compiler" ]; then
      if [ -d "$path/compiler/$oneapiVersion" ]; then
        echo "Found oneAPI DPC++/C++ Compiler $oneapiVersion in $path/."
        echo
        oneapiRoot=$path
        return
      else
        majCompatibleVersion=$(ls "$path/compiler" | grep "${oneapiVersion%.*}" | head -n 1)
        if [ "$majCompatibleVersion" != '' ] && [ -d "$path/compiler/$majCompatibleVersion" ]; then
          echo "Found oneAPI DPC++/C++ Compiler $majCompatibleVersion in $path/."
          echo
          oneapiRoot=$path
          oneapiVersion=$majCompatibleVersion
          return
        fi
      fi
    fi
  done

  echo "Error: Intel oneAPI DPC++/C++ Compiler $oneapiVersion was not found in"
  echo "any of the following locations:"
  for path in "$@"; do
    if [ "$path" != '' ]; then
      echo "* $path"
    fi
  done
  echo
  echo "Check that the following is true and try again:"
  echo "* An Intel oneAPI Toolkit $oneapiVersion is installed - oneAPI for"
  echo "  $oneapiProduct GPUs can only be installed within an existing Toolkit"
  echo "  with a matching version."
  echo "* If the Toolkit is installed somewhere other than $HOME/intel/oneapi"
  echo "  or /opt/intel/oneapi, set the ONEAPI_ROOT environment variable or"
  echo "  pass the --install-dir argument to this script."
  echo
  exit 1
}

getUserApprovalOrExit() {
  if [ "$promptUser" = 'yes' ]; then
    echo "$1 Proceed? [Yn]: "

    read -r line
    case "$line" in
      n* | N*)
        exit 0
    esac
  fi
}

installPackage() {
  getUserApprovalOrExit "The package will be installed in $oneapiRoot/."

  libDestDir="$oneapiRoot/compiler/$oneapiVersion/linux/lib/"
  checkCmd 'cp' "$tempDir/libpi_$oneapiBackend.so" "$libDestDir"
  includeDestDir="$oneapiRoot/compiler/$oneapiVersion/linux/include/sycl/detail/plugins/$oneapiBackend"
  mkdir -p $includeDestDir
  checkCmd 'cp' "$tempDir/features.hpp" "$includeDestDir"
  echo "* $backendPrintable plugin library installed in $libDestDir."
  echo "* $backendPrintable plugin header installed in $includeDestDir."

  licenseDir="$oneapiRoot/licensing/$oneapiVersion/"
  if [ ! -d $licenseDir ]; then
    checkCmd 'mkdir' '-p' "$licenseDir"
  fi
  checkCmd 'cp' "$tempDir/LICENSE_oneAPI_for_${oneapiProduct}_GPUs.md" "$licenseDir"
  echo "* License installed in $oneapiRoot/licensing/$oneapiVersion/."

  docsDir="$oneapiRoot/compiler/$oneapiVersion/documentation/en/oneAPI_for_${oneapiProduct}_GPUs/"
  checkCmd 'rm' '-rf' "$docsDir"
  checkCmd 'cp' '-r' "$tempDir/documentation" "$docsDir"
  echo "* Documentation installed in $docsDir."

  # Clean up temporary files.
  checkCmd 'rm' '-r' "$tempDir"

  echo
  echo "Installation complete."
  echo
}

printHelpAndExit() {
  scriptName=$(basename "$0")
  echo "Usage: $scriptName [options]"
  echo
  echo "Options:"
  echo "  -f, --extract-folder PATH"
  echo "    Set the extraction folder where the package contents will be saved."
  echo "  -h, --help"
  echo "    Show this help message."
  echo "  -i, --install-dir INSTALL_DIR"
  echo "    Customize the installation directory. INSTALL_DIR must be the root"
  echo "    of an Intel oneAPI Toolkit $oneapiVersion installation i.e. the "
  echo "    directory containing compiler/$oneapiVersion."
  echo "  -u, --uninstall"
  echo "    Remove a previous installation of this product - does not remove the"
  echo "    Intel oneAPI Toolkit installation."
  echo "  -x, --extract-only"
  echo "    Unpack the installation package only - do not install the product."
  echo "  -y, --yes"
  echo "    Install or uninstall without prompting the user for confirmation."
  echo
  exit 1
}

uninstallPackage() {
  getUserApprovalOrExit "oneAPI for $oneapiProduct GPUs will be uninstalled from $oneapiRoot/."

  checkCmd 'rm' '-f' "$oneapiRoot/compiler/$oneapiVersion/linux/lib/libpi_$oneapiBackend.so"
  checkCmd 'rm' '-f' "$oneapiRoot/compiler/$oneapiVersion/linux/include/sycl/detail/plugins/$oneapiBackend/features.hpp"
  echo "* $backendPrintable plugin library and header removed."

  if [ -d "$oneapiRoot/intelpython" ]; then
    pythonDir="$oneapiRoot/intelpython/python3.9"
    # TODO: Check path in new release
    #checkCmd 'rm' '-f' "$pythonDir/pkgs/dpcpp-cpp-rt-$oneapiVersion-intel_16953/lib"
    checkCmd 'rm' '-f' "$pythonDir/lib/libpi_$oneapiBackend.so"
    checkCmd 'rm' '-f' "$pythonDir/envs/$oneapiVersion/lib/libpi_$oneapiBackend.so"
  fi

  checkCmd 'rm' '-f' "$oneapiRoot/licensing/$oneapiVersion/LICENSE_oneAPI_for_${oneapiProduct}_GPUs.md"
  echo '* License removed.'

  checkCmd 'rm' '-rf' "$oneapiRoot/compiler/$oneapiVersion/documentation/en/oneAPI_for_${oneapiProduct}_GPUs"
  echo '* Documentation removed.'

  echo
  echo "Uninstallation complete."
  echo
}

oneapiProduct='NVIDIA'
oneapiBackend='cuda'
oneapiVersion='2023.2.1'
archiveChecksum='525ad544d059c44a9752037d810c5e23b249b2545e6f802a48e72a3370c58a17c3da4e5ae202f35e971046e3fa67e6c8'

backendPrintable=$(echo "$oneapiBackend" | tr '[:lower:]' '[:upper:]')

extractOnly='no'
oneapiRoot=''
promptUser='yes'
tempDir=''
uninstall='no'

releaseType=''
if [ "$oneapiProduct" = 'AMD' ]; then
  releaseType='(beta) '
fi

echo
echo "oneAPI for $oneapiProduct GPUs ${releaseType}${oneapiVersion} installer"
echo

# Process command-line options.
while [ $# -gt 0 ]; do
  case "$1" in
    -f | --f | --extract-folder)
      shift
      checkArgument "$1"
      if [ -f "$1" ]; then
        echo "Error: extraction folder path '$1' is a file."
        echo
        exit 1
      fi
      tempDir="$1"
      ;;
    -i | --i | --install-dir)
      shift
      checkArgument "$1"
      oneapiRoot="$1"
      ;;
    -u | --u | --uninstall)
      uninstall='yes'
      ;;
    -x | --x | --extract-only)
      extractOnly='yes'
      ;;
    -y | --y | --yes)
      promptUser='no'
      ;;
    *)
      printHelpAndExit
      ;;
  esac
  shift
done

# Check for invalid combinations of options.
if [ "$extractOnly" = 'yes' ] && [ "$oneapiRoot" != '' ]; then
  echo "--install-dir argument ignored due to --extract-only."
elif [ "$uninstall" = 'yes' ] && [ "$extractOnly" = 'yes' ]; then
  echo "--extract-only argument ignored due to --uninstall."
elif [ "$uninstall" = 'yes' ] && [ "$tempDir" != '' ]; then
  echo "--extract-folder argument ignored due to --uninstall."
fi

# Find the existing Intel oneAPI Toolkit installation.
if [ "$extractOnly" = 'no' ]; then
  if [ "$oneapiRoot" != '' ]; then
    findOneapiRootOrExit "$oneapiRoot"
  else
    findOneapiRootOrExit "$ONEAPI_ROOT" "$HOME/intel/oneapi" "/opt/intel/oneapi"
  fi

  if [ ! -w "$oneapiRoot" ]; then
    echo "Error: no write permissions for the Intel oneAPI Toolkit root folder."
    echo "Please check your permissions and/or run this command again with sudo."
    echo
    exit 1
  fi
fi

if [ "$uninstall" = 'yes' ]; then
  uninstallPackage
else
  extractPackage

  if [ "$extractOnly" = 'yes' ]; then
    echo "Package extracted to $tempDir."
    echo "Installation skipped."
    echo
  else
    installPackage
  fi
fi

# Exit from the script here to avoid trying to interpret the archive as part of
# the script.
exit 0

__ARCHIVE__
�      �}xE��;_$1AA�
�\T�3���&d&KI0��tf:��|1�
�~?p�廝y�L�R���T�l�Կי�WJaG�+�
�{����u�/�Yà,�ړ�Wh����3:ߪ�εEK��xQ���O����ésO/]���=O�~ƛ7����Z��򘒞G4vf����<���q��,�|bv�Aav?IR~�������M2)������(��׌��gK����.��I��K�0nBv�CI�+�|��We�Ò���<���II=oJ�s�$#�����\���K�3[R�ے���d|���᩼��~�$?	%�^�H��_�|�#���d|�d�"�!I=�$����%Y�Ԣ���yI=�K����~�I�pDR��$�j�d_��3�(�Z��?�̫O$�����.�NI=�_$yvJ�
tk	�Hh!#��z�W١%C�tc���T�-5�qK�Jb_��%VO� Ww�S�n/mq���:`��uR��RmVׅ�z��2�=�#�F�4�o���,-ד���h����I�����ąu!�X��D���JVWJKU-�6J��L��22
$ wđyX�����F�0�����/��@+IR5I\WEF=�G��]:������ޣ�a=��l̃�B�d�z�$����q���$�k�:�H���)25�PP��	�4�et�|�0ZW�f�Ҕ��\�jh�t)��Huf�E�F��e���㗐0%�x!�:�ú�	�[�۵�-k���>NzC֔�
!T�2����H���h`�n�䢎�ΐִ�i�n���Г��=��H�����"���D�K�z��oy �G�u-�(�K�
$����#O&�E
R��S+��S��y�U�D}�'���r;��u~�����ui ��k�@_��돑��i�t޵��Cﴖ��l҈�^�'�z������I�� V��X�Pc�jB�v�jg� O�{�X5���h������k��\d�Ӣ!��u� A{aE��3Wx�1C�X�r��a=�8��BA����d4N�aF'�v��k.���Wk�*Y7d�C�tU���t�W�x,
����.��AK{#���]0�gڢp�CCw�2u�!��f��kYґ(� �����u�z\tɨGS�.,�Z�3F�j�DD2��tnA[E�0���(���Pt�H�kd�ZVf��*I���k|8�a}�_�� �V��F0p��kXI�(8'��z����̩��ҭd��`QC�ZY�T55.hP��飊*~�>r���p��N���p;3�5/k3p�B��4�.�PfIwM�.T4� >�ٙ,��2�\��b.��(����H�<V*��6<�<f�����3ҫ��'X�q���>� �a'y����_'Ӳ���i��>o�Mϕ����Ǳ?t+%�i�i���e���m�?��?O�_��>i8V�������C�bJ�������y��cZh}G�r�Y��Jn1�<6�f���ԥ�|�2�خo���(��s|�{�4����!�*n��+�<pm.͵��7��HG�.�{X�Be�����=������ײ�ʣvY}��_Xl\��B��=V����V��v[�2��.�
~�U�L�uK�g���6�
�Ժ����w�)��_��+��j���d�90���b�����-�_��6���;P*��i�I�߅�E~��ߋ�_����WQ=	�!� �ߊ�Q促����ȟB��ȏ �����q�
�+�Aށ��r�_�t!��ȗ"?�2�Aށ�7����,��_�|-��!_����������9ȷ#�R�y�{�n�]�Ǒw#ߋ|%�������!_����"��y��B�F�� �{���!��#�*� �[�?�|�G��G�D~�o@~y����ދt!�/Eއ|��;��&�_���&�k�oF�������#�G~���/G~5�-�w#ߊ|�6�{�_�|?���oA�v�!���߁�N��D~�߃�]��E^E~��ȿ����!�;�?�| ����7�ב?�|'�#�w!�����H"B����/C~
�?A~�A��i�҅�oC��G�/C�O�w ��2��mc��6��mc��6��mc��6��mc�W�>,��o�{����_<���6���|���ΏV�K��3Gv%W���s��;::��q�78�2~�s����)���<������\�X�<��2�0��<���s�+93���B�9�K���6�B��'A��/��9_�s��s��s���<��\�s��s���|���r���t�������9�?�+!~�WA�����9�C��gB�����9��<��|-���:����?��?�9?�
���
�sn��9���9��sn��9��s^	�s���|'��y���.���
�b�j����s��9 ~�A����s��9wA���!~�!����?�5?�0��9�s�B��c���q���Z��s�眄�9?��Ϲ������s��9��s���|��y=���~�����6�C��7@�6����ut������?-�ox���^/pJ������n�Y`����x��b~�<E�b�>ד�	����>"�[�&���	����~Z�'�.�V�7�^����u�W	�&p��^��\-��g
<]�)\ �0��/�q������	|@�}?'�n���	���U���8%pD`]�U�	�,�W��W<G��Ox���|��_��>.����5��O���-��?!�v��
�Y������J�6���
<_�j��<S��O�X���%���}��|D�~M���9�w��;�fNj�y��5�Oy�����$����X�r�\D�^��@ܶ�[���/�O̇�"ͷyԘ��8���\R���F�Y@֮?�S�x���R>)�7��T��Z}y8Wq��A�mR�o��D��_Q�V����48�q���7p���8p�ypvy����K��+&]��H�z4
����	�o�d-U�������(����{�$
b�AEH4DH�&E�M� 
�2�@ی����(���"BH $��
���Ca_BؒW���$������w�����.u�[��nݺFMc��v�C�r�����id�)��h����r�c�n9��j������@�M\6l���%@W+e�2�;p��̣/��sl:c�����4�k[:2�Uĺ��l]@H?G97Q�!_I�l0]eeSI�v��:��1�B&m8`NEu���rJ�C�w����bVqў�K����WtpS��W�)n�S���R�W��Rl~��%��(�v�T��(L�^V�-�1bE�b�z-łk��*٬9�ĹX"M����%��v�$�wZ. C��AM�K
��^\�������z+ �<%�������	���36[;?�=~�y�h;d��+�<J���&��>���`��bB�|����c����6>x�C��}Ч����S����)�}�/��A�f���0:�sNt�}�Dy'�
0�'@�v=�~�7�i"�њM���Q��	�\�5+�ߤj��Y(@q��m8Ѷ���݃%m>��V�MKn�!'��*x!ku��k��u�2��N7H�������RxӞ�G��Vt\��8^�"��:�+��R�iYhGX�m�����՚KL|\�oN�m*�;s.ܼ^P�v��Ѽ�7Y��_�ͣ�~���clL�DA��u܅h���M��c��Xm��-��N�b�
�T�V�	�?�. X�u����о�4�g�־�-<dZ�I�j��'�X�-��z3,��m�� ��&���M	4���K��(�Ƣ�֖ʊ׶ Q�R�л�۟Z�3��G}!X�)Px�=��f���'����f��cHy��V-�%B�2�.X�+�Mef��Ԯ���ǈ�g��J�}D�gp��s�T�M�2W�1���2����~uFh9~\d��o�D�J�f`��5֦�?��EC���\�{U��\��E�O_�����Y�L�eZJ�Ꮦ������Y������m*�j�����Β*<�vcy����t���E���ǹbm>	'�S�\������ ���E �
��`�Ϟ4M�����&�Zԫ����&��h�M�w�<eK�j�G²��'���j���T|NT{y�c��6����|�s�ֶ�'�:�IǴ�:�%+��!�8�j^�j��/p���#'����i!&���^BoKvX䙈�:�~ӚA��e���=�)�w�+��N�d?60�Wb*p�f��BJM����7�&����58NӻO��)����$����!��k�C�m.}kH������8B� A�Ch��-B�
 ���`Y�Y*����;�k���V�ㇹ�?����4��J���S�oq�b��hy��.y ���lF��RЍ'.���k �I�@�H�wܤ2��D��������(
S�v+g
כ��2�9n�u�Xև�����ﰔW��ǜ3�#��!��������� ]���6!�чXポ?،'5��^R������N��B,$�zݘ�H� �Rp��"QZL��v2���fj�@��v/��6A:PʣP]=���_�ic��?V�T����[�D�n-Ay�*�M{��,��Yճ��Y~-똬��([��%��kMh�v{�G���kΟ�1{��R������>��� e_�B{�⼭uQm���ڛ��9�^T�Ϯ�h�q����>�����C̢�.���J�X�OFm=UW�����_Q�Ϸ��|�EhB���Z��aj(z4��˄�5�uZ�V�??�
�V� ���:č��ܻ��T�e!�Az%}�]ॄO�Ϸ���� �y૶H�Gy�g,/dc��.������BuH7\�k�����?�������n��u5Մ��d�������rF#��� 2n|ʂb<�e���������B�0� W3���5��=�,+���M�
��1-Ɣ<�d�o�h�]+k�'�"�k@���ͅ߁������)�cU���R���	�/�ύ��g`	�'��{`K���9ҍ� I��DS:w�����^�:X�b�t#��m�����>T�>�I(�a9*8�A.(���E��r\|�{�{O=?�;1��k˾�',��&#�A_�=�k�n����K���û�1�k�B�7�+'W
tB�~`FkNӮA1�N��U�9��)g	��HG�ڤ����I�����q'<��<<�ݗc��u����4�~����+�5�Q�ԟ**j7U��Շ����-ٟd���:�_7YoK�/Y>p:|����5iz��
|H�����Zߘ��K~��{���H/wt����S��]ũ�g�4� �鴿T^O�K�Rc�/�e%>��B�c�<�J�]>�n���5��7�H�
�h��F���~���������g�Ԕ�s?����2��=�0�#�0�o�!�����E��)<]���`U�Ǫ0x����]c�@���*����|���u��E(�G��2��_@���/�7��Ő�ls����G���˂/�_��K"(���~��~�_U��b͓��v�1Y�=�-�����/���IK��T����V%��	z�%�oRM��?��k�`���	�x�I��.�
<�;���\�*��6�	�J� fl�ߢ��rp
�����R5,�fm*��T�؎1RﯥJ���c�?��U�C�j/G՚�cQq,N梾��]�5mD�M���XӉ]��"�j}�j�ت�a�7\C����~������|�ub}���T�Wj�����r�s��d�%�����g�|KU��('=	e^��SP-\����r��`O��^Vr3�������S��ү�8��'�R�z��\���W唜p��"@L�_�������U�V�s�6Cv?�ٻK�N�!�uIS�ƚ&8�;�~�Gن�Ha����Sp�CO����g�e����̆
Kf��;:G�)���r�8E�\�}^����=�QJ�BV�t��O��7#��\������A�tx ޿}�x�[d5��Q{�J������� �U�2|�YV@:�%���UӠ�t�s���^��������w'�q9�&#��Z�\�w���q�����������_5���&V���M^K��Q'd^u4�ݖ�>�{Z��Q����Kv���1џ:�h������О�nrY��t�d������[ ��#�fBE��{�7�į�5퉐
�|/�(�3�RN9���OK��g�]�xi����+֫v ��ƍ��89��)+�\�hd��5 k���i7����,�;��T[����Jo�W�U���w
�����7[�R,M���
�'�v�X��*]�c�N菷�@����;���t���WR� ��s�>iN?�`���� ����m�epg������&�������g��\ �3�������[B��|Ҳ�s�Ϲ�1��~��"8��_1��v/�'"8�����;��O"���0�Z,��:�������x��9[���X����x�_��;�G����
L^A���10/���Ɖq��<�J�T�y���	``��%��G�r%��N�����'¹�\&H���T��i���i�u85R	���I�e
G9pe-�v��2�Ns���*P�.Y�$g\ �(A*����|z�i��pE%�&UhAϤ)��tW�J��
Gj*���%�}�p#�#�;PM�;<6,�"ړ�/������Qmҕ5���� �������N�>����m��P��^����!7ny&R�u��ڄ��㈅��P�]-\�z�9�)m&NE{`NF��ϴ� �A�5�~��-f���B����j��h�O�4U�gUT���_��_�?Z=|�o[=|���z���/�W���wV�^��U_e��z�.����/������^[�_�/�/^�g����Mtx^8�ڥ�`�x2=�P�_��x�u�A��k������kE)j�R�D��QV�;�|*����&-χ)�=Mi��aDgK����R�@r���6�|�Ϸ^�����5'�]��[M�85ܼ�}��FC\6�VRY�����o_�/Q�	kd���Vv_MQ(oю��aԒ��"g\���+�8���֌��A��z����>�Q.��5>l"��^6�.$72)'���{
�w�:(+�ޅ�[G	k�-ak�܋���N�:^���Wh�����4�KX����h���::ο��M.>��|�1�_jN�Ͼ*KV�k�/Y9��C�"��,]5�%a���]���E^]��5:I����Tƭ�x�Z��؋aq�3���h���<�~^eh#��Ko(0N�W�7���g$g���h��/�Щ(�K?x�<���;>�����u��:r�]h]R�o��$������b�*��Gi��)�P]S��"��V<eI*��F��\R��;���;~��d�M�g��A�ⴣ��a�S���l>~~�8�Wr��i�����|v}��lu<�^F�W ���/ܒ���%{
I�;I�;J���7�5+QO��/򒤻Z�uo�$ �wD�D٩���L(��.gp� �l��*E������G����r�6ئ�Yh����e����VY����tCO�`�8;��I��'����� ���۠��?���R=?��C�,H1�����E���;��ޱ���(:�/~��~'�P�|�������u��8�i5����ߨzx�oL;c��-��F��PQy���T5����9�J�C�����K��p��~�۩#��!���G�P �B�=�s�͟�6˂T�v�u���$��hz�Zȹ��B)W���Q��ť`�"/��b�ip�?�s��濅}=#_�R��"�JHu�n¼�a���I
 �:Rt��ߧ�W�6���ImL)��~\��Tf���_����<u�a�A��z���쀬?KGP��э)e������t�LX6 j\�7�G`��,l�������s�n��Ծ�R�rb%,���P݅���۩��N�ڈ���5�1���c�Ի�#����Q��f����jNE��D���o�{�{nA�D'y�����L�����܉АFw
 Ax_I6��>B��!¯��?�z���䕞�)n��u�r�-��t�'�A�G �N��KN��V)!��y	����s����t)a�)a�B)!d��L��@|iK�΍ee����mx�+a���{RVv��B_#��N�����$�
�K&��@�-�W�3�A+Y��쒊N(6�k���h �c�t'�$�A]
��Sq�!��rNZ�O2���Q0v�%it7�ߞ�&MZ�aK���rz��.GQԍx��7lE���� ͫ��f��V
:�R�<)'l���]4���+���R�l���͏�óV7T�|̻M@�=�s@&��l"��z��*�A�+\�oy��`��~�N�iq��BAK��0t+�pPVk�t5[K_Dv0��\kC���(c����Q���z�@9�Z�����9L���X-�c�����/��p�*�p�R���aj��1F�n����������{S�b���(��������ًܾ��W�I�
�ˤ�G���lp���@������'��aM��|#l�AgH9���r惓��h�ԒC�
�Քr�b�!]Si��P�#\��o�~}�Vu1r��b:�D���6K������e��3F=��ᡑ��
E�½�-�fu�CV�:�_"�Q./��梴:0M�9���E�]2	z���\
C�v=o�ˍ}/�6�����&�WN���̿�cH���j�o�/����0��6��o�:��5?L9C
~�Uy�cd�O��Q&�݈���I����c�J��	���G���U�%�G�CK�nRi�(M���?09O1��LC_
���_�ls��U��b�ݘ}�v&�V�Î�x�ahu"-�Ѩ���{�[��Q[q�Ə����g������~n��
=�aFk�%�������	�߄g���?��/��0���1��Ï
����z( ���Ռ�94Z�B�͚�=:�ҳ�\��z��Vd5�a�Ϩ�&P d��o�5��t�ri:�l 63)�S9�1�w�K�ӝ=�F�����,y���״�<�F^��W��&h��E��5RN]b�[e�� 	�7ሌ��#��g�I.i�lm��'�ʇ��ym�ri�~��#����^�AF�Q[��E5�L(��
�g�sx\�nǬ�*�N��(�b巜��*�)P����!�4�	�"U�����0Gǌ�Ҥ]��fP��ϫ�{A��2s�M-Y
@�#S}]�X�[=�"�m���E��*���������K�
�m��OӞ�d�Y>�����v/�A�z�����>*:�c=��N_Re�z諦W��Mȧ
���H�N�ܒ�y���8�C�3]mx
��ֻ��*2T��⟉�-���FA]�
׹��W�6��ބ=�כ���Ǽr����ݓpޫ�1d���ɯ`$@a|<	�3ݗA�ŝ5�
����x���⋣sW�e�8���I$����>��-����&9�f��n�B����n4JV�
�&H��Ac_!a��3M�>~�C�Ɗ��n>�U1�+|�`oT�T/&�c �;e��rO0j����m����e����q�r��`�EO�Q��!��`WY�}�۫l]n1��PMka^�\Y��k|�Ӥ�5�P+�Ә{^�^Ș�p�Sp9� ��Nry��0���i%�9��Pz��iiq>N�@>T!�Ҽ���G�s��q�t9ح��ɱ�B�s>6�,�q����E��2�(p�Zء�}��^勔�����dڋ��s|�ƫ��b��R��{����T���x�M��վ�4�ڼm̧��;��z����� 4w�L��|>�%��K()��Z�>-{z�\��:"������h�͆�	�Al텙	�B������J]�z
D���C�1����&t_9�){�I��O�y�73�c��W����H���� �o`�;:�	k2��8�y��vy[�Ѳ�5���ޙ(��?d%�Q�Pb����[�<
p���S)��uo��;	�ͫ�D�%�]I[�����f����[d��Ѻ����Lj�� ��^��Ւ��J�HD誄�9�|LQu���>����T*"��������y
����н�a��`]�C$g�|,�0$W��5 O$m	�V)��9�����a��0=��e�+��!�.�J����k���=ź���,n/�Ȃթ�yE ��&. �t�_T%"{��m@"
����*�L`����bVKS��@D�ψ� ���ճH����*��t&��ր��A�Zjut�)��*e��W���BYa^���tu&#��w��6���J���<�}L�� ��S�J�'��}��gA�ͫ�^�dx[Sy����!��G���
����0#
���׳��*�W��w:�j���V~S��?�p�����'�?�{����[^:^}}]��Gʭ�}ԕ���Ԟk��S�̿^�A��z����n����t,4�����n7������X���DzS��b�D�v���iZ�����Af�F�M�ݏ�|b�{����Tb��18�]�X��A祅|-������{X������ѕ�m�G�_��w��Ns�T��Ҷ,-X���&����S��/��r�"�
���5s�Sק��}Q^.�6�)�p�����Vބr�_����!���f�(����L*��Mk���gA���G�Q������ɰ�N��zl!46�;���=�I���AsOg�Aj�rh5�/�dc����>�3Ӱ�z���)ի�ܶMEU�%Ђ�i0�h��`D��1��ew�O�.h��Џ��A��V�J���du8�#ʐ�/y� 4�,�)3PF
�V�/�ˣ�7f�9y������LV��0L,0�'���dp*˄Oh�� �䧰M�=Fܖr�!f���x����SY;�C
F4؝]H�(J��kwЅ~w�o h��	���e*��x��/����;����r�7�ϤI�bjJ�b"u��12�Ĭ敩a�n���.���+P_;�q�Ŏ\���I3�K�
0<�3n���\��`��'TL�����G�Q�.$����c��6^�CF��k�
�����n�A@�ק�%�`F�\p	XS��Mw�g$��a#�p,@Su��8CP'A��H2�g���qR �V"��>��x�F'O�3�,=֪Py�xt2�`e���t\
�a� ��"�Ʃ�;$��z(�8���V�#��6�� a�F�_���j �� ����BB�ȧ�a���6%�p=��y�M"
?N�E�w�	Ď��p�e\�!��8�I��z��p�A--M��D�O��@��=J�j�t�ԆNB�2��6R-�.��/`�@tb��3&JAR��L�����RV����M�S�7
��@�2�O2(�^HB.�]���Udɒg� 1`�5dx��� ���Կ��(���ֱ��T($UyL.��q�.j�_�%dnXo��0�rD@��W���DD��?#��͌���>aR�Ĳ�������?N0�OoS������o���m���<6�fc0}<^Č.ɹ���% ƼF9$W{�Xd�|�b�ǀ��C�kN��Z���:�[h������@�x.[|7����=e�KN�W��Ⱦ�F��z�n���Ԫ�_8"\��I�Z����q�G��g��ap-�g�
Ǹ/�?<�NC{�K���L�G1S)$��ݓ���|�	C��.>\Ɗq>&�P���mw��S���;9-C#Mi�����Q7�� 5∂�/M��M�(�n�����X��blQ�/Q�}-d���D'�)�g1&���Nމ����J����x����l�ʃ4sޤ��eT00ǲ���01z��t[�{J��k7�Ei
|oX��G9W�<��㘊�6T�G�)Ӿ�[0�^ܒ�F�_��,&i���8��vm���Z�rҫ\ ���ڇxk�Ӣ��L�j],�� �{�A'+�n����C���~��1�k��½:���'A\nK��e&\��\�KT=�9@�\<%�(z)	z)������Qrz�E� }�dxؔl���1J{�vV)_�;���F]yDoD<C�UvF�AΈ�����]t z{1��b4�u�seѮ?^�����bw.���n�?���K�M�/QqW���/��Z�ؤ���U|�b���h�P34t�����B��ou��Wh�Ɛ9��~I�A�~$�@"Y��}=��;t�O�~'�4}9��B��*@W�E{K_�L�^L�������Ǐ�?��?|ʟ�"�C�k�M�W@�:	���G3<�r\H
j�|�&~db��vk�[�@j�ad	�Y��h�Ϭ$�,C�����5�$D(�Q�MG�2�G��?��G��AB��H<���v=��R��B~��u�q/�#|��w4���ضΣ��g���6�4)���V!��t#d��(/�����z��Gg�7eʢ��zg��YC������#,�܌�#ˀ"� ���S|H��ڬ��t�#�|�p���<L'����88p+6�|�%(>�5R�#�D�R`#p����w+�<~��S~�|�\��U��x�����۱�A�iL$4�%C��z+�qJ����{K�9���-�����Z�6�e�5�%?y�
&R�u��7ż}@_�v�֫L5�)�h��Hˌ��������,9�2�v�eg#�L��(����gF��E�����2�{d�P�� �B��H�^`�jf<ne(�et�aZ�^�\=5;���bսn�ȷ:"_B��Ȏ���T��)�?�c,�(�1kT�
�\L);�R�%Rށ���{o�o�����%��[$|KW�����w�?��0�]�aW���6�^�r������stJD�h����<܆�Nn�6p��YFfN��;Ī�P��KĪ��N�`�j��4"�$�P`��x�t[Bk����ь�N�+��D\��K�.D߲7�����Y{���L����BٻS�k������HpZ�a��ǡ����x�E\�Ւ�u"T���dc��3E��X���"���Ϣ��%��]X����&�W**8�-ǹ�rz�c+Ǻ%���!i�NA�o��dE\��%hg��n׫
�Qa��"�}m.�-�^�ڳЋZ�!��̡_|��d��G���#M�M�C}՜�ҥ�Z�"�\��F���=����\1��=O���a��L�i4��B3&�<�D��<xn��<����K�� fi�g�m8����`���2��4��G������Y��v��쑅���bA��q�d�K8݃"KP{��+�`W�?����y����eqҔ$�:�e�P��ܕ��2�|ϣ���h�?�O�h5��V�
ަ=��baR�c�Є�ONE�~H�/١�?N�x[����Oͨ����}	��;0���b+��˝8�a������`|�duB���!o6	�m$�nc	b�)�t".ٵ���x�xa�v#f����ي�̦,c�H��aX>�&�����ܒ���Q��,Nt=�:q����C�98	�&�H+	r���:�-�T��g�g6�V��8��-�q�ԉ��|�)j���dx9)L�[�+�J/�O�)
!Ԏ�j�kO�ڿ<B��QE�j���K�-)�A_5p:��^�'���{���C3-rW���F�UVNC�U����h:@7��f\�oI}��ߓvZr��$�\�ԧ�G�z#~�*���,�ȯDu�qW�=-�-`��W֡��[�*��=��Tɴ1������6V�nI1A&��M�U��_�״�b��'N��v*l̠���J���W�q��~|j�U"R@vI�?�2��l�T%�j�H�d/��5�rG=�ք��]��[�L�<�|�����
��5�ˁ8�л��X����}=�H3ѣ2t_�0Ɨ,��׍�aƝ4oj�pS���M� �Vc	M���w�c�5����5�g�-��]'��ge���RR~�d��RQ�:�Ѽ��T��y����H=�uFJ#�iv6bx[�߇�}�q�vWZ vws���v51a�������bH@��E
�}t_]��|���D �:1
�lnsjY��e�{�[�� �.�sy�qؤA\ˀ�Ԥ8��SY�؛v��辪��+�D5��J4M��1���ģF��+b�n��f!�Òs�
l��^�Yv�I���$�<�2֡~@d�)Ś�vu-L��`�T�M����������;���ڔ���������޿*�&׉=x�<��_��(��x+�r��Ab7�bY��F�"�5YT�o��
@:>�L΍��庾S�}��}��L�pR�����F��T>��&�Q��T��Xr|�ִ��GWQ*= �l�U�8�e	�Z��4�|}��ߊâO��*%��V�qbU��^{�W�˴���%(�cP)+U?�o�MR��J�7ؚ���ټ".�,\%xu�v�-i����!� ���Ѧe\����=�����S6		>&|[u��W=�o5y����4�؍�h�NY���R �L
7���f1FBS{[^Z%����j�_i&�&)�Rڳ��/�`�1x������AQ�p��
���Џ�v?O9̎T��l}�ðr�_�y{�玅Ӗ�Q%]*Pw�.@:���v10�<]Y���|�3��]��)8�:����U�����o�oZ/+�^%���Gΐ�;7.���I�J��=�%�oTH)�>��l\��ȓ86�w�%�a�m�?*Cɴ	�ǫ�~G��6�׻k�E��QXª��td�Qް�ss3�r�TuF��h��Z� Y+뎚�C7��n|���t�	 !�u\�f�P'�����z��
�����g������,��N��I��G��Oa!n�H�#$P�B����;2�}L�E�R9��2ͽM�ŭ�-i�K-M�JO�A��Kٖ�'���HQ�"t�OIׁӻ���i癝�fCR��Є�[)Tt��t��4���20T���.��G>�I�)�t���r�i�j��%� �{��O`y��F�]�DF���E�OR��-������
<��� �F��5&&G��1/
y/]�Y�ہ�(_a�����OG��˝�ѡ���t����8����B��N�"̱��5t$�0>]1���ܧ�3*2��(��qI�~�?����1�:��m�!F��-p�X��݋��g��Tk�1��$"���R�p����<�6]�
W��线��Ѕ[���	m7��k1��K���Ō��X(m����	����C$�4�f�w4A�x4�o�S�~4I�x�nFN��x�Wh�է�v��iCS�L��i��*��9Mنs�,M �fi��?�/M���s�T��D�:��&�M��_%�O�i��Y��h7LBV�׆[�4\�y�~_T�u���˵�X>��#�}T��d*�n�+Q 5��"M5��e�W��4�Rko� 7ܥ'S�KC���bFp<�2�F1n�WiߊE��z��w��Ƚ�d���@%�կP8�/��F0g@�NP�cPSzpLEIK����R�a)�6}>T�z���*��ht�a�c���Ҳ�j3�י��!��l�?*�kŤ��9o���܁$�;��?Z�4z��`Ѻ�v�������v��Qe}��mh�S�a�Ndi�!��R馓�<e+�Wb/uii�nt�v�\�i�C�����=Lqt=|ځ��M]$)��*�{>U
|cg�X���y׵���n��uE�i�-���M|%�_���j�[��@s�2t��[�j��a���!��';�u���-�}��p�*��t+��qO�}�O���Da;=��6�ߛ	>�%W4�sk~O3��vĺ_�5���V0���CR�D�{�w$'?���=6ٷǫ�M���}��o�r� /��#�XG�)>����_��q�HVk�[��b2Ŗj�r�ȝ�߯9J�MWw�q�Z�٥����I���9��l����W�(4-
��'q`�k��T�5�T��$�L*ծM�s'�������Y��$C:��^�_��]�I�r}K��N*$�F4!������捛$J�ʺ�z{��@Ǡ]#?M�3n��JBHvR��أA��i�R��{�-���&K��r�E�ei��ǻA�4�s�o�y��ڮ�f.ӫ>��kq���:�)�f.�S�B
e'9�I��]���4��%��l��v�-'�PHl�Ŷ&�p�*[x�0[��'*��I���3����A�[��}�������6;����k�f`�m2#A��~�Vve�s>���r
k2��o���Z�cm�H�rd ����t(3��W[�����%%밮�zՖڀ�f�ۋ]wW1R]/m
��j�Ghe%��$��#?7E�xv�^�[��qzq�>Q��������� ����4����яO�j�w{eG��"��h�x�7q�h��h5�����0���#��6�
�G��:G�C#͗qK9u��O�U�1�IT=�m��<r��Bz�����eq�6��y�Z`��"��ș������]�4���ɧ�Kʇ�qқ�I���1*7�sHXz�����n��_�C6�k�pEk��H�'W ��!oATҘ3���1ņ�
�ޡ+s�.�!0āxTF�GR�G����wr�I�S,���/�Cޒ�U��+�d9��i�,���8�Rq91���H����B`J2�W��r�����!�71�h0��7d�fn��]B⺼�����x]������d΃ɚ��iYL�! ��o9M6��i���-�8)��:������!�k'�kk�ks���ך�ײ��א�ug{�y���޾��%���/@#�]
��[����8+Pw�r҈�A�ڣ��6����������̢9<��<a.��ٻ���/L6�ea���$6�?O�K�os�5��@�?���r��*�vy�w�J�{�T�N��b�o&LP�~�S��
m��H�!Қ<I6��;h���	s]b���R��F �R襤��a�;��ЋBܖ����}��>A�+��D�f�YQ�gUu�-I^�^�8Ȣ�����BN��)�I����t c�%T^��/�V_r���Ѝ��v�����-�v0�-5o�}o���0��c�z�aJ�q�h3U��C��d_C_�@�r?
q처����FR��8�%�托���4�W�������G`��~�Y���\�����p*����i\��X��p��d�FQa�z����>�οs����8���>��3X߬r��QqP�;XW��,羢?�2��OG�p����-ܴ�*و����	�5�[2�[��'��`m�m�F�4]8M��B�v]s\:C�]�Q׵���ε6�7e���A�@�&
c&.�~�r'���V��+�G�J��C��O�/8��"� '�xW�)g!��<��#O�fߣv�:#
�qE��f� ���i��Ӊ"����6n���5ܫ�<�K�f���׻lCKy��YΫK��aK��K���+(X*2�d�����G����)MA��G��������K
���t�7�2�.r���}�9.�l�s\��b�]I��s��l��l�����g�	�V<m��MO�jT�~��@�Jo�F���JV��	=&Ǫ�N$��)dz��C� ��J�.�9�`z�ح��8.v}\��<G���=N��rx#����;����
<Ғ�~xm�����Z��T�;,xg�'�/��)�0�5a�FiA7�W���2,��v1���lP�JF�Q���D.�>��s��I&�A�t���oTI����P�|Ɋ7��=҂�����c��7SI;��fX�b��vx��uͿ�K�65ܮ��]"(�ޖ="%�O{��� c0w�k4�1��ى�(��z���
Xw&s�iۉ.�����Y8��X)>�?ɫWl�����Ey/��Wy�.;���G���U��ԛ�"����S�fy��
�C�r3v@�7P+�B�j����v���#b�/؅�a��2�|�]bp�O��g�倖UF�c�y����O����?��<��c���Az,&�K
�A5����9�4��

R��Erq���8Y�aL��v�^���,@�kx��1��*��r��,�@
� d�%�K���?�)@j@�

Ȼ׫A�0V�.őcb�9���=��9.=�_��y]Ъ{9:;�$�5��R�@ɕ%$�G��xT��B�G{��8���xEc����-�+B��@5��ė'R�U�mr_���N&K�*�#�n�~�t,T���S?����v4>�l3��7�,�?Z0����T�6��f�O'VL��,K����5�ץ��/��Uekh�:��:��:4����L�U�#O�����=��2��┒n�ڰ�#f��1V���8t}���{��l�!��f�_o��!�����l"O�����L��S��c6�D�4&�:��i��6HS���
Y��6-֘�8u#뛦���8��q���V�j�nV#�ܝ�[�7|*��"�?������n�H��ml�)"F<vЈe"�(�`\�ĀL2U�K�+�O
$N����p�����	k����(��-� ��9�d���d�y�ep�!�͗	k��Mf*�K
��#���#���A����F'�k���w?K�霦6��T����)�|o;��)�k#��`pj1��`�Z�͇W��F"-)͊�0o^�i��'큺�w�ѲU���uS�!�k'�kk�ks���ך�ײf���Y�W�Z�
��ͧ	᭲�/V���ૈ*��fG3�.[�ec$3���{��g�e�j���@L��{��XU�.��fd��دܢ�N�[U�=�?�eSk�Y��=h�{O��B�?����u�yV]v�I�}�;����t��r$���g�b��+���WgQ�cꎕbb���:�)|lɞ��l+��t=��U�؞���$g7�W�e�{k���@�F�2����=i�Qn[�4��OGķ�k @�Q���� �6{
�ӫ�6��ިH�2h7+�YWQ{�OW��}�Ք�ذ2�a�u�X�e{X�4t��j�uA�A8;]��RN�`VB�مn�[� 釸m��W��vu�0����R+���r�y��O7��0�
>�B�O7��Ǯ�׻��g���?�u��h��677�;w%u�|>]�!�yc?߻Y��޷���%;�y����cY�\ݚw������h#��/4B���4�ú��&�,�4xN�`X�����h�y���ú��|�c��w���\s)�a�CK���7nuhm�s����$�X��i��h�B'�6�������ga���ۭ����%��/�����W��h��^ϲ^l0o���O6�zq�v��
�	�kݯ�[\ːYvu"*�Q.������0��G��Yv�3��q���\���G#���F��V#×&#C4���n�"��>��F��iG��*�vW�V����=bI
Ѿ0�<M�60���_��Y@�9_��(�b�� �V�GR��c<
s�����纗���^�f�h��R��	s�d{��³��aa��ٜb}mi}���괾��k���{�f��h�rn�#6�pAV֋�=2:�Ţ�W��1�XUsd�N]��T��d��` �&
@�������8W�r�N���S0g��rg߷����-{]-�۔�ف#�P�Er�W�� MM
u��K��f��9���qt�)3����Of�3�˪��H�b�`=;�հ�����?d<�l���9�R������\�{ҕCZ�Sf����X�d�~��#�Q���FX�O��0��}��B8�����:5�;�һA��'	�
��s��:�z�&���d�B���#����f�37��N2��'�|�U���d�;�;M���e[�H�U��Llb�#��>�5�K�����|��I���G��W�(/}f��qO��? �g�bT�_�A���_^����~y)��v��K�_��C�|�dh��/��3�r�(H��H: ��e���RLgB��P��8(�gw��	��ք��(�i��ؽf�.�X�y�s?%�@r}���
�ہ��s`A}J��0|���?ھ� ��"� �e���U',��HwZc}]j}�2��-U+t��u��u�x
=G���3�>��T�j�d��p�}�u~���Ntc�&�e�í���1���ޙ��8�q�1� ����)��{I�������E��rޤ��p`��"M�q�u�ǸC)j�7��F�6e��)���t�DZQ�æ,�Q��*
0��X�h;B�{�s���da������[I7���J�o�>�)J6yǱôGˍ����,�G,�~o�D��M
��6�����*"��<Y���*�?g�X(d�� �
=��gM,Y_m��3��J��J� 6�!S�0��$�;�n�������R+G�RHa�����`'�
;�k���	���=P[��;�P7݄�?.������<%�,(Y�g��e�-�Y�����������;b�@y��\g���p�����)B�����?нB�f�������N�09q�"N���h�5�Y����x�l�i�TW�iH��������A3���ԙ<��L�b4�+�=���&��� �\&#-Ed�X�ϟ�(�e��)�)��/EI9��d�r�S�v�Zi{��I��݆�_z��s2���&+�
������B��9zs�?B���ΐ��1��p�~3���W}��wo�o�#��H`��x��C������1\�k�h��}69@�����pOB���>7;�)d������N
�^���>�23y����x�*<L��+��h�E�l�P������G�A���&���=p�}�;c���}5���q���
��_��K�F�c��}&�l8�B5N���;k�ڬ�f�~�(��8�`*U�y����FB�4�m�%�Z�m�g\Xn�1~���_A�)?���j1վR�
�����Y��'M�1r2&��F�  �G��P�Y5;GVNy��6i�Q�^��f��҂�i�.r���v��t]��9]��.+�3�֦Qy��v��w� 㘫�T^d�x(�b�޵�ȗ���` ��\d��lmaT|�o$~��e�����l p���[o]�O*�ް��1�r�y�@�Բ
���F����i��'����l<nai�'~��(o���p����3��ت\�����u\��\g�g*q�M��\g�Y�s��ת�:�5�3�i ��%�����L1�Θ5f�3p:�2M�=��lӳ�^�]7�����J���\(+=�-��H�{�f7r08�^��9{���k?�����f3_��76����ĝ�0�18��b��Tb3�m3��wqo�ށ�͔�#���2�y��f^5��/�ٌ@�
��F;tw	��N�����0:~����W!�������|-��J��w�3\k���u�0?���������!�P�N� xu��GӏX�чG��ia~˜���&r�Y��0����c��9 �G٠v�s�'�T
w����0.�{�(������.���W�/M9I�R �%K��2�5Q���u��UD<����M,4{���������]��-�8�R�D�:{�,^�`�FpT4�O1st2s�f�䘡M���n?B�TJ&R�'��1w,���2���@3�Y$CިsE]$C�h/պn2��q��?�M���U��f�?vf�8�����V�M��N��o��\q��+��~��T3�xu�RN}�8&1"�c���Il>r��xŏ�`�Ɯ��$/U���	�L�o���}���򿗉�m��?��}
�+�Z���&�np �����K����6�D�)���,�?F�n��%��3�{��m�f�Bijc�<Ns+���4�q%� ^�����3<mj�{j#�khu��	L
NÀ���ѿX/�+�,7
3Ɛ̌�$ ~��7v�
Ǥrz�8��a��%k7�J7qL)p�'0SML:��N*duy����'�������`>�.��� �_����I����
�^�}OA�B�3vq)�Q@/_�l7.��r��KS��Ȏ>�g��; iIϪ��n���k����g�0��������4^��'w�V���~��r�K��������1ޓ�z�Q���˵Ds����˵�=�nq�X�7S�֋�� ��	�/�
S�V�N����{����ALF��|K�����o�N�^٧d��N�S}�vb6�yՖ�a�ʬ ^��ۮYh��K9�l|Ov�f5q3��Rd��,d{�j�v�=�3RϤI�2ㅱOJȜ�Q3���#�l��`�t5�ݓ�֣�,ik�c���we��t�N��+z������ ��EW�W<�R�_d��Z#����Dg?��ِ���Q6�O��~b�*�����Z�3ܢ��Z�s^��h9:ch�Щ�˒�5�˭��C�Z�g.��@�!�����2ڗA��� �DV~�jqD�	(_�Dx$I��)��XVN����H����η�O%c�v����v�r��=�0�~�\R��/U���H�c�!�>i�h��&d�)_�ʄ����8tFv><�9�����Np
ŧ�d�����<%�y���*zT��N��q6�gU��%Kb؆<Qo��F6��Gi���D��ҀM9w�Ot�X�����h�M�����
����h1K�X(��_n��%�Y��N��D��G��j�D50߫~"�bhPl�q�.��49��>���d]I�C��T[i�'�YՁ�>BX� � }je�(o�%3��Z�j/A	P861�W@N
Z��}�儋��$����v�]��of���N�
3�Q�]���Z�B�/�ċ[�y�ɸ�
eul2���n76����+�Y��k���}��+���
3�B,eKj�8z���C�X�fA����d_�������i�;��28O��+�����E� �W��7eh�'�2L��I�Vj�΋I�:��#<Jgb:���&��I����\�Y���L�E8y���!�T���e�:���d�nv?���u`����u�7
,�el��=��|ś&��(���X��j�v0��}c�����FX6F��	F��9�� � =q�|��?/"�~/cp9�>x�(�3�� �v�C���<������~�����< wep��BJ ke)�^,"�-��/�q���/���
���Ҽ�L����4}�6m�71��8+�,�g�U���<puty�L�î%�%�0�*pmj����C2]�tgX2ϰ��'�v��p�`D
/�ɼ�w�o�hU �fa
�n����x=��%��p��LA߈���0�����3��Y_.(c�\O�a^{ҏo��~L���T��:�5HN 
��D=.�����N�C�����wè���nTO�˅W�d)�X�PxW��'�d���ʻ�g�x]jKפG�6^��ʴ%x[96�8:l�R���vDZP����RB_WV��+�\w�H�i��ٖ�⽌�� ���!� gm��ĕ���\)�%\��i�`�� n��6gp)|Ҿep�o`p
�7��5���a�a�p��w�>-��Fц�A$|߅�2���ϝ�uU��x.�Lo��Ӎl���2^��yF*���ó�o�rQ�wR����0PM�n�}��ʇf��f%$����M�r�n�k&�e�`�gp,�6��e��%M��lx^�Iy5�7�l zO�	��G#Ce�����Y3��զ�սZ���u?�����s�qK9t�*�!^	�Eף�Ib?�Y&��7��iC>�j�$!�+_q�{�E�I��EQ�l�x��P�;q��p�|����x%����^���J#�6|��9�q�o�'9Ukܠ~�2c	ֈW�W�\y�гy����+���}�b�g��~��d8;�{����P��ȟD#)��w��y�� /J�0Ҕ��0�Q��5u�_��1I3�7�t�*A壖x�1-d���w�vgX��^�!�|��7 9���0f�I�'�_�c�C*�hN��	{(rD���W�kde%�m���|��l��k?�M8B�ߌ��ܦ�j��i�i^�k2IcCK~ �UF��E̅����/�PW�ދ�#�A�+ \�(��.Y-�!��;g3�ۤ)wk��^��~��q�'�`9�l�"aG[�Y�m=>��M���G��~dv�c��w�c@�^�;��xbX�3�$3m��v�{@Z��R���ii[	��!��w
�`(¢��N��	}<(ސ1��
���_C�U�����9��	���w���
�͞�%�&R0s�V��#��i$1GK2:�J��
��Yaj,'z�M*����P���)����NVT�˞��5%��!���aH}��n���E�gR���2�#<�LC��;}}
@>p@�v�萦�jB5���xM*w`cK ��`ym�ȟ]slto��H���K6p3�o�1�NM)�D�?'D�PR>m����)�r�WC��
�<9����X����,�+���������5���x�ZCT
�D�{���&��u��.r~"�j�M�t�rn�H0+�`�H0Gz
%�'@�}�RK���$\@{Q ���S��[��[�7o�,n^�q����?���D��"�"ηH���ȷ�u�"1c�6�^�A�Z���:�~�
ެ=�6�5Z~�h�^q�J$��H~)� u\�R�%U�R�7͏�K��QvQ��uW���|=��O	ٯ�e���DD0��駾訣�QG��z�]F>��(���@���+o�����ES��@q��h�l�{��W�Gf���δ�n�&�����䘿Z��T�/ʣ4u���.�U���I9/��p�5��C�+t�O�[��R��S,6����_�Q�60]�tJ�2xF54߀g�<,���H"!��t�[�������_�TQ^^~~�Mk�O���RPr~1���^ �F�R�fe�m���I��Jjkk�]C�4�fx8�!���+��@o(>���T����!|MI���U 5�.��D�wdj��d�iJ��M_������䗤�n�� ���ew�����,�hP�G�L9tM�ӋD9q�r4�M����'6!��t���J9?75S�me&ʹ��jp�p
�n��b��*j�3QŅ��*2�m�Ґ�"�-��	|�0>Kt|f�ac; #P*M��1�����3���h���Icj�Ϣo5��#�۱D�c����5�#�,�+��������
kCx�' :�Mu1�mE�B�W���Y�r-�Y(�<~��Wb	�|�g�ϷΘ�T���E}�ی�Ҩ>��/��}.�{�Z��-r��\��\׊\�8�|�K����Ƅ����-X�<m�r?��������8P�r��;'���Y�����x�
�?��"ZV�e���2��Mi���|��"m	,%t缼tЈ'Ff?��\i�I��a��G.�$�";A�^���M�Ro����݅B^�
(%R�xֺ�5j8p��R�ڼ�2)�!t��3zm��N
���yfR��Ƨ��#x�Qh�;�鎟)����n�Bh[�H���m�Gͨ^�+��QҜ �东���ӣ��E`|$i�(MC_��q��F�(����������q^���{]��G�����q:���T=C�x �{��r����\���k�ã>����r֝	2��v�J�e�M��R���L��"@���E�rp�U
����A��1���he��x]Y�y
2�NknA�B�v�\�}u��
�`���Wltp=}_�s��4v#I�P�,M�34JNC��\��"J�C8 �_bu��<.�7��+��W��ko�2���hE��H3���l}��Pe2�#�7�d�4���5���գa�Ch�E��O�E���P4��e�Q���Ăt���,�x��l�o�O9傲[6�W�돚V��\gsQ�Y��^�W��z��g�*!Y��q�y�\Zb�`~	�<펛ux�Ļ=�k��J�m��>oG{���Q��2&����F���^c,<�UPHf�1@��dͯ��bvYM)gG|pd_��<�鹺T�>RQ\R�^�,Q�.N�w���9�d#E�EuY��r���@����h�
�\c���z��
��"��U8E֥6! zD �{D�=f^W�����I�V������v�s~[��`�V8�ˊ�K|3�
sxdr��yA�]�@����c������ٽC�fwq�ǹSȊ�t�`��>�,JT�C�f��"7>S���qm�vO�Dq�h8&jh$�Q�F-O$���X��|���ac)�����56ғ^�ÃC��
I�%LW�$d$0	M��t��˸(�K�\��()7�a�z��Ť�UN4����0�T������
����b�X}�I��2��Ұ�4��I�	F%�'?L=.)a>���u���ݾ�O��
O~��E�i��4�x�c<=h<u1��ګ?�m|K0�n4�Ouv��64�kJ6�M�2$_���1Z��l:b�7��&bؿ�~��m���W�t@����n<��z4�-��k�\���!���U-#�TF�0<��5s<}�W�	_X�LN(N���%<ȃ���-��:	ڴQ��Z���i>ts�(�#�rq'�J
�5-��z�:���=��FҔ96��x�Κ:yx܅>!��K�\{���b`_����v�]�m8�{���^�`�S
$T�$�`���h��TXF�R%�)�/��H<0o��O<3bX�p��=��������O��������Q\9�m���>LW���Jؒ�&�֖#Y^{v-!Y�3펤�wg73��61��Yr\q��K~���\A����\�3��$G�`u�1��"�{�{fg�$'�Օ�j����_�_w����k��E�������lj�{���[��"rLI$��2t�w��/�Q�r`�|Hp��
��
ٮO�{l!J`5/熗�i^�B �o�	ŧ����`����=��>fɭ!inl����E~m�3�tH�h�!����Sg"1c�������D�r6�~n��6��f���D��M���D��Md*g���D�����L�l"�h�Bb�6��m"��\3�D���sOY-�YȪ-���
M���:;
�q>�겗�t�_e�"�k�Pŉ�����{�T�+�����V�!��72� �y70����|a��P�Q��Y�Y��_��A�(FG���)�!����E�1:��Y�*��̢�ct��e�a�>}F+,�>=_��D�=z�ޒ��o/<tr9q��aڛ��#�)v=$�~�7�r��-��1�s[+8]�H��-$���@���! �������������=8rPx��������6��n����W$��@Z�g����Z<�_����Ȅ�K��zr��6�	/rKN�4��Ef8��4�g�w��1��e�'�^Jȼ� E<���-ϽC^Mf^+��?8���eVvU"�/��+�E���/��D�4c�H]B|�&Z0ή�D5�{������#���x��5RN���u���e�|h���mP��"�y=¢�dч�/���
t��$�$ǻ��B]/�&zx�:`�:�~�r�>+=,�ɣ������9�H����I�'�:�s2*<~�%���*�5���e7�H����587Ô�&�ѷ���3�?����6������M�z<�,}�y��Z�̦��H%ok��Y��ƍ��6�3Jޔ瓋�Q��qG�.��`���i��l�����@�񐔹�����/�%� =��߳'=k���4Ƙ��*��g��3$���%��&�U��
�? 
|d�s�������������i��f�2�H�YWG]�����㇥Ƭ�[��%gŘ����wg�*�d��6�\�- �o>wMLw��/��2x4�8��B��V�Ͼ/�Dy���R�Wy�5ӆ�eP�ïC�S)���"
@I��V����2�tsRM��\���՘_,�̾'�5��|����\i15���&�{Q>��Gry�.���s��M����\�;��&S�}N�m����
�|6�q�P %O�R������!������Y�V��oݰ��ӝ|�*��?�����*��L�L9��6���z� ��[�u�-����N�cc���o�3��g��k�Z����o���7ln���u����e�ꕟ����'G�+��C������'ˤ��l)�>nh�@�d]2�Z���jVPۀj�F�;s�rXZA'#��r��o�쭞j�&��+#�8��|��@|>��b_�Q��������z����e5�#ݽt���«
i��E�oQ�'�?��4�ӊ��5��k`עj��b'
�(�y��+��-�,C��V�f.]�P�D�Z#غ�� 0n��7$�2-�h<���sfG�<�|
o��-��p@j��oF!Zlb�&u
~`9+'H$B���fO����#�"��Z��FDX�1|�%M��p4Q���^[�XףYf�iEe�im��w⻢��7b�톚2b݉������shve�g�Y^�|DSa�g�Xnt��m44K��ծ%J��E��Q�w�wu�xXҁ��~��|�Z--	�������9���N^K��f!t��n+�%�C�~��k����S�D���o��oE�0M�j�+4�l������L&q#�hQ������{�xLd�!2�(è�s��+낳�N���Bv�NE6h5t�,�'iA;`-�Zr�*W%����Q
���
���$Ί�G��uyP���KЋ�}�|c�V�qE�SnhQ��e�??|�	�<pQ��f�~-F��r�"�3��l�s��}�Mrѥ�q��2��ӣk����&�'߈pK��K	1����쨻�v'~k�6�ṹ���g
#W��zEd��0�a�PB� I+�"*�'a��e���E���H5��I���W�K�:<k�sc�+��-�� ���[�2঳���UhL��-������R;(�'\|�-H>1�@�N� [��aCK ����&�p.��q�j�E�H!c�r�궎
'�T4�E�tC
)��`<+g�m�I�������q���KȪ)b�f��<��F�S�j!�>�ZM~�.���QiE����c�og�"!*-	���|�@�L�VykXJ�6.��YW��Lg��i������tPz�^�%���VM��0A?�~���\+�T;-R��P��r��fA����U��R�q���\
��ā��phW��ϔ	���qґ�����Ǌ3�c�8��z�����)�o��6���O0���W�=�K]�Na
�Pg���yl:G��A�hڦ�	��2 6�c�s�$�3�t�K2Q/��o[�t�9���o�i�)VMQx�ǁ
����X\��[o�رnu�/:�B��(��DT1�l��J�Ĥ�%d�N������������z��Sx�׎�����-�W�ϟ9#4� �$�ϐ�g�>��](�/3��smNu��M|��3U7��څe�E��J9�E�g�v l��cQ�Bِ����w��̣9�pb��5�����Q\����Z1^��MX�ld�ŖA�2�J�/K�#cl�+V_���V2�q��_"c�Ss�R��ser"1u>ι�#9�P])�D�\���brx�~�gfwvvf��SU�ӯ?^�~��uO�L���l�N(�[6b&�p�G�K?���U=�M=����)DhH�S[,���㊒��l#]��^��[��4k�jf�b��pEw_obˈ��l��0C�-k�j+��҈L+�j���LN ܸ�˗�%��)oVgC�FZ��f���º󡉓����m�ni��*��q��Z�/J��%
e�*��IG�P6��t狨߁\�9]�C&�9&q�	Tn�ߗ�9FGKʕ{��>e��>�ו�DL��|i�i��k���~�,A@>�|�8Q�����~S��^�N�CfL��ȣ�=��,�Dt��S<���(s�־�h�-u܇h� ����/6�"3�ԝ����ˬդ&�0��u���N���dqn?پ-]��m��u�ܛΚߔ�i�"�D�e#5�=Y�C��M��p��U�S��cx�OH#qr��9h�DX�N=�
�)1�H@}@r�}E_�w�O15&�n��g5�T�b7�^�:�)�Ӓ�=��鎺���ۣ�{��n���*S�+Py�t_@<K`|P!�[R��xÅ���wYh��.	��f���kq�@�KV���U��i�h��Ҫe�&�R�{W����
�G��H�>OQ�n�"�B�PG"�C�.wS�;�Po���Z��^��u7w�j��VΕ�
����Bp�z���UI��		ӨJ����1$��f�'q�K�li�)OKC��'�`%|��"f�l������or~��������&�]����J�e�7 ��_
'�V�B*�ҧ+�J�½Zܗ�ҥ<l�%v�Z�{�f�uk�/����˕� �ڶ��m�\���YHO�FL7���6�C�K�kŶ�DB���VKI���#�?��mq��c����*A���=��(�����G�E#�K�� Ҟ2բmC:�P*��%�%)OF+�X�������`�]��mʑH��AյW
8/��[�y/�uʁ��sF���CU�����<�?���|���͜om�|��ƶq����)�x?�a^p��G�����c��
]M�N[�!
�T�>V?
���܁9��C�'m�ؤq��g8�Z��V�����Xt����$�jHȨ2!�ʄ��%��.�LU���%=q�����U��?WYS\*~�Q^�ڋ�g���]~4yǱ�y�l�w<�^�[�e��$�T1}N��)���ys�r<i?l�t�O/�u�O7+�E����9�uө��H�}b��L�9>͞�A�z��D���jv���E��=������Eʅ�FA;}E��ڭj]~���ꪖ��
TI5>�K�����V��Iጺ�2;���������WM�?sz�kg��>������U4}n�u��\�M9B�g��l'L�N�S��΂��
�J����*b
G]	�v
2�D��ȶ�[��6�'�Ӿ=�`G�;�}�O��9�m8��,}[{W��8T9M��g�&uA�A�!��WW�V�/vW�~\�;6�|�4�/�<��=9���i�I��S]�I��DW�����Z�G��a���_!�����ު�~��m�e~����d�X�N�#p�5�|)��?d�ب͝{�˫sR;v`�l���SjG�F�#$WV��q����=�	��N�V��
�$z�_��>�!G]a���>=s��{6����W;�?�YPL��{�z1[3?������j��
_�,�Q�C��R�?�y���C�^��=G�iU�t�Ɣ�Z'V
��=F��ul�)�_0�_G�}��	ϾڹQ��1�R���(ʘ@ϫs��Sɽ
�r�UU{��O77���ع5��q6���j��1��7��Ƕ�0m6sZ���msu۠������j�2}�~+C��Ϊ�sd��
WFsy�7U�p�+q��S����.1�}/g�i}�s�i}���\����ѱ_�c�m-�I[4��<Uk$׶%�k��7�����Um6�фf��~�����p#je"Ļv�s�C��������ctf���$�v�j��4�$��!9���Q!d*).(Ðm��o���*�W�4x���O�#q�.��3��sC>K�+<��X���(cntD,0��nĉ��8Qƚy��O�ց�q���a��?��m�Cx��o3]Nd�1YG~�I�;b}<�@�v�T�ڣ�O4�f$��E�;�ƈ���8=�_�D�Y���I̔����L6��b?uy-��jÉ�1�u�L��2���>7��_�����W������6�l������Q�}ѥ\ݓ5z�\i��%~�V�S�<��(6Ж(�Q����"�_����b���Sc��oc|2����W�%��GLx����K������~����71�!��oB�2�n�N�y���Kwh���!�����P����.�'�x����
�I����o��c��d}p�%s�6T�tRg��8��!��Uk!l+ƈC1��c{%>G�Y(�cę�|y�h'[�sz���N��[b�m%I����9[ٟ����D�L��G��W��x���xŨ�K'��G�ͨ�������G�#owoc��E��KU�$�6�N��>����)�?�rEJҩ�74���{=�����m�~N@��sS����/ʫ�.:��s�N�JU����0��O��a�w��J�z�zv��̷pt.�9����fȻ��('�U�#M|s�+��� ���5���������`��8�ph��,�RU�����:qhw�]�{�^a��B� �����Kț�K������2U�-F}]�U�|�9o}�ƸOC�وE�D+S�М��I���~�?U5��?�^�w��?�}'v[/�8���"̶���g�s,ҳy1G���}
6�Z{������Cs���{��_K���/������0���6�j�����M���[��j�o�"��JUk��u�>�\�z�R������ׯ*�08Y�yG\�2��T�s��&2�E�{Tm������c�L]̡R�Ls0��8��WNG��ę�O��$SMtݺ+d��M����߆
����nh�>���}:ߞb/U�E1���m7�U!�#�����9�8�GT� ���ꅳ��>�#�p��8c�s�d�F�=�jO�<[�
��,{�?�\/�.��Fx�7g��LQx�B׀>iax�̋�Nl�/g1�2Xɤ]W����7��8��@���ߌ��|ǤÁc�� �����vy:0�(*��.�)q �8E��o��y��v����x�!��aH��$A��"}���P��tg��3���O28퀳 ���p��ဓVX� �%���$�v�2�a�&��u�=X�%��٤̰I9�l���OY�O`���&o��螹�5��C2T���*]�t��.i1��\�T��c6X�~r���5N�����.*�w�I��d���l�e�S(e|�`�P��[��b�Ƞ�&��p�&� 8o�޲�u�Tn�Z��Q��D�C�f�=�>{
������o��� 9e>�=�ܷJ�������]v���n���s��U�-�v�	��e+�.���Dc�>4L�H�y�y&si��9��n0t�Gd�$.�r�������ݾ��e�gy�q�n�^����ͽ.����^# �Ϗ�]��(�9�@4c�M�'���<nm	�����_�< �2`9�{v���b�q�E�(A�v	�����c�ho����!���=;��=�� ~����OJ�1n
�H�)�J�7-�S&�JF3�ge��$�~)`�����5�t���>��hC{{�ۗ�,�_gA��~�-IXhon��l^���h�Ω��r������;T��gYh!u�7и�͔�X>�cgm|��Kw�N���u�t՟�����s
!w��%��"��'e*�+��S�d,���}�-�?��uSql���y���yv>K�Sv�@�J�I�;�%�R�Y��y�2V�"[��u�5���}���Z�HI�;�\�Z�)�g���.h%�N����W��;i���$Mop��2\q���_^�W�;y��]�z��Ȗ��{��͎ ��}�QK�J��Y�wbq��9�?�xjlP�c�{K2��[ˁ>��u��^K�7�ſǎ�m���f��0��M�5o�Y�b�7�����
v��8��p�>�,���r
���;�*<�ݢ���<�h��������=�ca/P�3��X����׭)�c�};]�;�Z��ƞ�Gz~��Oo�{n��g6��
�D�mF,@���x��m%lV]�c�����(�X�Fy$���6��r��g>�-u�E<��*��� �9���5Lv�'v���jK]�E�:��,�.�[�ߘt_nk��@v���Ϩ���e\�CJL�Mk��l13Y�&+��tZ�J�a��J�C�\�p�Cs�G~\o�z�k�B6��f��᭳���@��#���s����@��v��v�H���H��t�Os���¢�ҩ�H�Z*��+�K5G*HǊM�Tj�x�7u��A=�h>�=��*EŵL��&����M�s�� ǅ`�r\X�"��>��j�kcX�{�ao�l)�����ab��I�si.U�����ywW
���q+���F7��։.x��ָ�|����57+rS����n������L	��d�zƖ��P�Ǖs�n{d��7'xes-aH�l��]��Xha�a�	~�Q��H����qks��gI�Y~
!�b$yIB	�9v�8�'Դ���؞�-��$!�x�(m
���[{��c�o���4ʑ�}L� e� d2�9 �����A�!�Z&�!]�nH��L@f �! ����C�9�<�T��!]�nH��L@f �! ����C�9�<�T��!]�nH��L@f �! ����C�9�<��6�~H��)C!��~��,� �d2)�Q?��
wU��x�o���Hm9�j��S���
����.�oV>��Z�������*�;V�wa?��}
����-��V���r��Ir��_
:�����,�VN���]&���S��AҶ�;��Ң˜������'�!�|��':�]����2�=�8��
���^�s+�,Ɏ��a���h��/윦S�h��3�Dp�5�{�&���Ρ��f5���[	�K�D�������Q{�=Q+?�w(�&��g��ൣσ7���e��P_)p�2���x�3�b��~O�G���6����d����}/g������}oG���ҥ<�f����6�|�u��_n�}?ƇVX�?ݟq��V�Q2ރ��As�H]d��������Ͽ�
�/8�Fد�=���M~����`��y��rV�z����I���L�iV�9���C�8?>����^�������׀O�q~���A�悿
�"�����[��V��Z��*���Mb����6��4_���''�z7��qx||ӫ��?��&?�����O	1����/��>����V���O��NB��܋@ş/}
>|���3�/�L���t�����W�?l㏁�7������w2��k9&�y��v�������=����6�
��<�r�+��������PN�V�f�~x�.j���A��p.p�������p�����9�<ӟq����N�<�sߏ����釀��9
?6�
����7�K�YC��v� ��3]G;���́=���K�v��q
��j+�����>\��)���]�3}
یt�	�6�z�~���_����>�����u�A9�kY{&�����K�^��<���������f���/�J��s��
ؿ����Owme弆z/�mg�xt�&��*�o ���d�T��G,�ۖ?�s�s�.���ϣ����Np�UL���fp�ZV���=�C�3�a�2��u���u�~���o]�#C��j��3���3�Ŝ��t�D�|�]�~6x�!�>鏼�{<.=��o�χ��!%�3��T��5�qkw�|�z�zg:{������b�^0?_|?����8�����m�q;�w0aE��E�(��C#x��� ������gpy�o`=�^~�u|��D�mqxx�ߙ���?��-�7����o�t����G�~2��Uh'&|+���*�w�9���b=�ϖ?��������s�2����y{�'x)\�q��_
�+>�g.�~��o��������+��?i���)����+��
�+^�s�?�����?	�˾b�-}��u��ck�˙+���|����;����o����"�[�?����>S��	x��_/�w
�����'�c��עzA����r�C����
�W	�%~u���m��+�O	��>��</G��D�o�|�����gx���.�K�����$�
�fW���9�թ��k\�~��o�M>$��s��y��7��������_����,�����~�����7�o�W�c�8_�/�+ಀ�~��_!�7�����������3��F��M�+Z��Y)��T�op)�+�tX�%�����*�Z�ն*J���V�kJB�#�=6O3ZՔ�O&�I�,�����h��ڣ�jJ
H�.#�]�Ha�a��4�ð@Y�Qy�b��WB�������B�ƢS���*J4ާ)Q-��1ň�I���`��o�F�N���b�/�Z�ȱ�MJzWߑ2�/E����>�%#�4��J�p�'���R�C��>4t9թɤ�%1
�/�`MJ��e ��uU��t����;=4:�V�&T+2)MQ
v-�[����L��h<��M�h��=Z[C}<��6�3�΢K�8�+5I-O��U��(��<5��F���P������skp�q��e1.����)Y�树����}��@�����Cd��[I��4b�aF۴�)�~y��yU�� MH�����g��\:��/c4|�5�(���b�-���J��+-������xʨ��H�p�1�i�=���U�Zx����T�J֖�;��bWl���$�K��6�I&�1�X��b;ͥ��L��y�r�]N�N�wM�S4%VxvXD;Y�#-I���-i������aE��aF�_��Z�f�zw* Xݺ]��[�X��F�]���X@m�ON��F#̈́i0�D0?���H��|�[c�g[d����z��3�Q�	+��)�K��-;����>�O����fęZ�&�;bQ�5V�x}ץ�H��B=qB=�xO�;��Y��mv���DD'��Pê���Q���-`�=�n���KM�!��$�jQ��xJ78�,B�a����}������?E�$��++����5NhIc�%2��=-$Ju�M�E��E'#�.5�XM�jW���G�/��7q�@M|;M���f:�!2�x�0`��"g�8=�H���2��U�����z�M�����c9(��cvH	��'����4�L��*\1�]�q6�ɜ��e�� Y��\!u��הp:%�R�)�)SEil�]@���E)g�<�thа]��f��2�B����a��k˷�+d��.�H�J¶F�9~[:b.-F'����v�>A�����M�#ݗ�i#�e�*L�U�P��'k_�!1��%���S�	��.�8
� <�vEw�k���g<�@�g��N׶ pp;���h����	�g�~��@R��9X���>��_���vH!u�� �4���)��'uJf���@���|CNa��	K�E�Y6'�c��0W������Z��E��1XSE��fhm�č��=t�_!���J֝��^q��� ��s�D�`J;�zp����{��u��+� ���+��㦭����ic<�)q<�ˌ+���I�ΊS�B��hxl��
��Y�j�d�
Z � ���k�9,�m���Ӟ�T�ϔ~����a�ً�R^�Q�_p,_ԍ��RX7[u��_�uLh����{�j��%Kي�}Mv*����M�� ��D�HQS�WQ;T0��]*Fr���5���f�O�G(�f�U����J��@�c"�[I�Wx,`Lx��6Y�f��X8�&NƘ�Z��g�1��90/1���m�y§=X[�l�(��Fχ0�J�q�W4�gM"�[����\V�_6?�bxA�����%O��G���N(,�|=;�}6.e�(�2 b�ޒ�b%�9�����5�sЗ��pn�=�[��_b�l�\�~f�V{����]|U�w5~z)�'��"��<�32�F4��}]kI"5�d�C�]��w�,|
ػT����_����=ſ��׾����KW�jTo���ŏ�އ����s�IE܊�Q6ǔ�3��mL-'���欳�2�Y��Y$��,e~�-:sp�)K�2���1���^�� FTP9�<��{p>��l��r�3v��M�y��i9���֤�2����'�����`�� �U��?�5�W凇q�� ��|g
v�X��r�@�Ս�X���d�QX�w���x��4��qm�?�9LY�������"�n�X���{�j�q1O['O����ȳ C�8/n*�S��_���9����|dpeo�◰����E��~t�.�����F"6�i.+�+A0��掽�=��T<�Rrw���B�EpA��
�=�h�$� 4I�8�f�LA��Bޗ@K��jB�}�X@D�NH����`��L�ܶ�m
Ԯ�n���b�*�`e9+y�M(i+6jcovG���G���Ș|�1��?�8�o(�*�����D��G1)����W8a6�Cl&%}ۓ
�O�o���[����
��������?������8�-!"7}�$�_�t0��+���L*Ϡ���VE/ƃ�&>u����,��|��|/x�W���I(�����t���2Y좄 U�(ٝ�>�F�b���w��=�ɨ�C�t<ϙ2���0G����A�؂��E�F��n+���y*�6�=?�c$T<�Q���A��4��ы�X6��9�=@q����2��a/ߘQ��ۊ�dK�5�m��x�l\�4=��J�p�
����6���	���ф�E�A#[蕆��V}�[��8\e�*����>�m�P0x��QD�Q�����U�F�-ם�F���Nu
\\��� �h�c���.��}olOyB`jbb�)�2ѥ˶����
G�Md�xP��*Ƌ����,��[���P~�_��\|�����$��?���T�w�	���7��o�q1^�/]|����O����9�U�^u���p�?80_� �m������7���<1^��]|g�a��_$Ƌ����?|�$����x��w�-}b��/�.���$<�.1^�'K|'�o�~�ǇoI�sX�.~}r���/�+(��e����51^��^|���������=����%��\���9�?!ޟ���=�vB`����7�uBm%�'����o|
Gs�O��ן'{������������f|:͎�����q>��F�\m���j�N��[��J�4�u~z�O|2�9+�>��|Ἐy�-��B�����|.G����?>�5��Ʊ���O������~��sMj4�RǱ1U%��[\K�)�d/���r5i�;�Hݛ�!UQ�r?}e���O|�={9ϽK xJ��-��wR.�)<;b�y���f�9�i�Sۙ�
N�����;Z��0r�i|�T��.��b>�p�Y�<�_�;����Koa$�����3���X몏39R�#h��8u�Xa��kW1v��H0k�>F!2�|��Զ��`:;%:/q�Vu@�Z�]����>� DM�lO2tSǕa�\{�m��\Z6B�	�ihv:%$|��|���VZ�X5ah�ڱc����:�	'��l�Lأ��K�1�ϩ$K�,4Y*�͐@Oa�
�e����r9�
�C�m��f�+�@�a{<�� {���i�_�$\$���&���
��_�eX�6��a�>�6����e����;%%�Ʃ�����&�v��LY�2��d�8�A���;�xkI� w _X����LY񩬐�O�. �@H͞�}�vBa��q����
���J�� �*�į*�$�<6����ρ���?�Y�<e0U���

Y;~�m8Z1�m^R�ԩ@Q��=Bc������L҅���>�F�
��X
��炬tŷ}DԔ�%*6'�m����
j���	�%J6�}fbAi�L4�p�4�� pt!l����b���
W�'Eh#�� ��#^���J�(��@�*vФ����n�ş'X0�adc�d�۝`���dk[A�0�<<��D0׃8��m�T���+#�|��+C�� ��r�t��Ѱ*��5K���j,3)�/�ȾtdT��7�-�N�0Q���9 ��wҊa�4p�a�����Kc�Plkc�<�w� ���z�v<���K��\�2a8!�lkĊ�je`(h[��D[�O,Őu�2�6@hGSDi�EKÇ�ʎN�8�ڷ�<4�ٳ���u�Apj[�r����-��$0TX�ɭ' �\�8`|�
Rh�R��S��T
%7����i�C���Bs�?��������I0qG� u�1�  4�A0��� z�`?ؽ�`��Nu"���J���č�0���5p�ø]�((\�H�b)��@`�;�D�/�C �)��O1��@�
y0Ӡ������3c"�����U� ��BC�"l$�H�8�1�.$L����I9pf�9���8�?
,f�tt3cN�W[��T]�$���== IJ�ǔ��3C4)�͆)�p�rO̖2=~G�|ɍ�D�a�oB���_gAd轹������^i[XG�10�)#�k�%}�H*)�Fz6u$|r\-�Na.�$��}}���#1�#l�n��f��[��C�:����B&���^h,�ǖnM�XAy>�	�sX:h-3����w\(^Ҷ�hnV��p����L2B9�3PtpQ�E_�&��19R\A��sAXm �|�I0�9��,� �Zv�`SpJ$~x��hԇ�� �3v£y�}ncC5A�U�Χ	�`Zt/*dGl���;'���T�]
p��9���\ka�3} �dǂK��1O�GaF��J�[���
JéX�'`>��o��Ǵ��O�)#�f�"�E���%z�
ɇ��)Șm���)��%�t�O~K��XQ7+�.{�mP�Ż�<��GW�%Q��ŧ*^ 3f�V`F���/Z�A(��4�ώ�����c��/�D��a3�ٶ�э0e�+k�V2&��cO	��ffdc��Ya�
�oD�s���/��X���2�m�y��b�4M9Np��i
�)�t�Y���<��{�S�6Q.�(�mPL���-� �.�|��<9�T4]�x�
N'�;�u�ɉ�sP��A��cz�@{�F ^=�m!"���+؜^�n�K�܃�(s�����r�����/h 2Q���QTG���k����[� �0|�-�!B	�zf�x�Z����FҀ#>K4<�'���c�3���Ǡ�D%on>�����s(x��Aa�k��s��~,7Z�Ќn=M�!x�`�Κ��s`��X�7��}ow5`���$[vE��Y9D����(5<�sj�_'t~H9�.0nǬJ��
l�Q�XU�T5�r�!i� ��	E���/�Nc�����j0�1s�G
P�c��\i�a,|�bU����hH���[c�1�/Im��T8ʿ�{���H�|:?�X�RG��y����R�jr4ϱ��w��T*��"����i��2uLD�����A��==�������WaQ� ���!��i����N�� ���
��G�� �rcp)u(axC&��P���#`�x|-}�l�#����;��ӠQ
j,.�r���*m(�R����?�����=ā_�l?���"7*ma�(ՄJ�i�#F
���n;A'��B��䟜Ud6C��+
s1��r�X�+Ѱ��6v
��
���y���H�@XJ�5D�1�.7�%�bp�A�Z������ɛ�Q ~�n��{c2�o����#�������ߍ�
�%ۇ9JE�x{�S�B)� xV~�\|����i&�'���0wBw%[��������{��_���G��?_0�!m(�ОR�j2��eK79��0 É=2���Dv����*|b��� �����R�ѓ'��	�3?�~����ОJ��'�ǟE�����f��4s\�C�q�	�{��͙�:�����:6��W��[���������M�cmf����
����L�e7��}fԻ*L�[_yZ`���v�?�W��{|����L��
�����������̆��]��Z+�wK/l��Y��ZOK��eZ�ѦQ.��ئ�T�6���F��k/J������K3����K��5��m����k�˺9Ϩ7����2��_}j���������_7u�7Zŉ՝k嬯��ˮ��Z;q^������b5��Y���� g)�&�Sd4��\f�ծ���,�o�ޤV|j�U�-����t7m�b=�����ʧ����J�b�X�֓afY�I������Q*���+>�o�֓�ea�PZ7{@�|k�=\��-�t�U̖�}x4&��"2�F�]f&��=ɿ��R�����\����]DqRn�w�r�\Y?֚���� �՛&¶%^�j/[���5�;��.]�㊭yu�k�\�zzɾ>��ye����ʪ���s�v_=t�N��|���r�~��R-�wO�E}�������>���c��`��R>����c?�7���̦�gv��ݶ]�3�'{�,��f��4��� s��`M����.�%o��걲2Z�z�]Q
����ZT���|٪�v_�>�3�fen� ���
����z�����Lop��x�e=�˶ut쮕�|
�fd�n�Z̊ /\g��r�����1O��cF�]�������O�
F��*�J��R�Nj�K�I+�����kKm�o��?����7��Ȼ����������pq�d���Sv�wOW�9讝s�����.'�˛ZNMϮ��������C�f�ػ5��+��U��u+W=�x{5�e���{}�-t�Ֆ:(ә5�����D���~hd���h����w~&��V�[Y�W+S-U�����ŢvSޥ����}?y?�7^۽ߩw�mgn���ow���[u(���J�iݬ��0�dL�c�r��������(}���.�lw^��w��7���]\��wW���k�J���~ߞ7�7rA�O�5?�o�٢����m��OgY��^�2�g��t�i�����������.�{���_�,+�t:��7����wgK�^��ܿ\Y���K��.zꍺ�W�����W2]}R]�7����+�V�\{���Y�3Оnm�kܸ��~�����������o����p~.;�zݨ�K���/O��s}k��B�)���δ��{�q����X������~h����������>����c�Y��ϛO�\��{��+g�hf��6�J�ݵj����R�}��`X���=�W��Y�����G���&��W\�,Y7�� � '�Zk�)P:�ֿ�ynfvWfu�`��Y}�88���+�A�t~��Tc=�( c��L�S���]O^Hݽ"�
\�!=4s�/4�$��0��5�U|�D@����1�^�Ђό^������*��
�4��=4;���x��gy�Ԉ��,N���`4����2���/C΂͵c�� �AҮ��c,#QR�F�f�J�
+4��I��ty	�Р2��\-�F^ޣC���ꛔ4Ǻ�lr�Y�{�*�N��2��X� ��ԣ.�i�;�V�$����*KM�Y4�no�j���
Q��#��EJ,�=����H����v���������a�ᎀh��5 Ɠ��u�1H
:$J��Y*�s�KRn볆��Q�<�����>�h�ڟ蘑
�C�G��k����\�{��V^"<�gz��͍uo�̺Uku`�>�1����6W����N4���H�tٕ��w�7��
�zHK��9P��ԇ��F��6�&h !�Ȋ�+����t+�����4�ɘ|�X&�R�~��@�X�)N�a����2l�pszكo�i���ƺ0x�B�dׁơ�mj�B��i�QMw�m��J��jM��ȭ9M|/a��i���V�}�k� ��L����Z�y��/Cv�Ǳi�'?�\d��M��M3�]I�(�'ͯ�Y�����ې:%,=���E┛~���F*,q��!����/�Y�xP
[�W: ���(�d&��V�v������k����r-��M�kJ?���Êrzm��#�L�3I�q�������pR�{�~�]G�O�x0��PU 5p��?�xVE��@ߍ�=V+P:��."E 9�݁2N�hxe����nͭ3-�[q[B�����x�]�v�*JS$����s���-[�GXB��l`g�+��:�n���f@Z�_Wu�m���p��$�Wr<�s�s9C�q@V���e� ;��	<�����b%�� ,T"�1R���+%X�F�[
�jْ����=J8��9�M��c
���6o2
������f��~��^���P�O����J���[�!nz�bW�ʢ�j�G�xrQF�r]�� ��,\܇ޭ��74�����(�a�3��Rg(�.N��'���eU_��ϛ#�>��ϩ�9(��"%�9^�p)�$�Ġ:Ft:�q�2�ܯ'bWwwgj�"E��\��w���Y^a����h�0�<
BĈ*}P*�M�	p7��i�g�Q+�Y^�؆䩟Ãg
]t�\��n���b�Z ,2�rQ��|p�u�z- L{	u�,����yU=[J1��	͡�>�P��DEǪZ�)
a��$��NsLN'����� Wݰ��qUS�G�6�|���R�IJ،Mr���Ru�([�ت�.
H,[]��8o?ť���
+
R+
��z�Qj�L�.}�M�5�a�3��_��l]�~��Q�r�\h�{iy�Wd�����7ԛ�v�ȱncq���7-�j�QEa,-��P�'���>q�k$@�0�p�͡��]e�&+-�
�ѯA�=ND����m�� A�{Y�|�_���`}����&i���<��[�Hg�d ���纽�-���5�Rw��HܶH��<"C}=,��h9��8�<{/��>¯�W�t<4��9>���9C��X\άa��j��p�O�MG{�6%�H	W���,��eR��H��yӮ�t9��Sz���.�-��w��Mv�!�~�5P�^�p��.��P�����ּ,�lk�lË��z�Q-Bju$���9��f�#n�'P�ʑ�5���u1��l��N��7��x��G-�'�8V5�S �6�6;<�"冴�����B9�%R��}�h��(��������<���K�C7��x��E_����9�2�:�d9�;ި]��2��N���/�2������ȫ$Q�S#
��in��գ�pt*�!���]JF����d�8�Yq�0��TW��[s~�ds EAV��IG-&���N��s����5��y<��/%����xEJ�l\JɌ�oj�4sﰈ��{|�@�%:z�'pN�}u	{�	�L[<����%�:a5�,1j-K��urW���=��lXP�b2�7��Y��sFK�U��o��(�r�_7����j�h*����M\V�ҽ ����
wJs����x��U�2���Dg��W�_����ܿ .8�}�ާ�x�	���t�L�0T8{��hm�2��K���>՛�2�n��w�%.��]�R�A���m;X%F��<��z(TPy��;i�j�
�޼
a�6ܐ�ZsXK2_o8T.x�^L0:�4Ods��b>�AmN����5b��F���Rh�Vv��
�4�'��:rЋ������u��w��j��j4���o�;ǧr��:I�Y��x��zB��>�)����vZ�k=i��̓X
�V�y���#sz�.-Ugp���k��_�� S�\���(W�P1ړ��Rkʜ�;}uy�|%m�������i3���Ӝ?۷r	�����͗/�y�$X�
�f�>��ʜqy�{�%�j
j |Č��/�cV:���ϕM��v|r��Ң��7�@E�6m�RK
o)0�
�b{�i�y5s׬�\97����ڔ�%��G ���>�F��2����~r�a�Է�#��r��hOk�Z|��Hw�fvs�Ez�D
YG��~'�@:�[Q,�XT� ��*>��-R��`����Xb�]4u4FWl�1ȋ�T�+n����!3� �����<{��s�V�����yZ3�̺�/���
ͽ�ڎ��!țE����J�o��5����S�_���1��'�j�ү?�����}H���\����㣰z}����`�	��24-��Ki!��LqGѯ�CN������M�c�\�1��y���\#�>?��)�S�jST�K#š1�E?��!45�\�JO��A*�@V�A�g+�ܬ$C ��x6U��Hm	�$��ze9<#$�Q��u�Z�8K�Q| u\7sz c�z���H.��������[�t2N�m}�ǟ�C�*"Z�iɳ��ӥF׉n:?B�TB�~\j/�B�&�z�z��,�{�k�tƝ�&$��x6�r���̟m��U�B�n0��E����>�]O��e��S�g%I�
&Z�_%tRd�F�4�@V)}� ;S��lӹ���B�_��t�w���0�7�x�q��iM@x�	����C��J��BÍ�֘�;e�
�d�ȼ;ݪ!�����BMC�i��fs�5~��W�h��N�E��1CۛC�IU�G��b�����sD܍Ġ7[C�.}�[Ku�y�M
a{���7H{3��ג�q��%\�]����8)� �țh+����ihК�=�]a��D�R�0k#��%-�įedd[^>' ���b����`��up��j���8�mG<�`l����7X�X�tG����{�G��c�1TF#�-�|�#i�f.��<�dP6
���w�4����*\��2d�%��̞�N�#�����i�P�F�j>e� ���XƲ�Dkk]j�PNA��Z7c>J�*�-٘�_lK����b�֭?��a#�1�6b�[)��Ł� $�5�|���$��nrt���]K5As�/z�i}e��3����T�2���A���n�����͵��
y^>�Ӵ�	9k�����j䠏_�B��[��R�����:�
��F��� ��\���+�?Ey媖�b�݊�	uqN%��D)����[�m���!��RN���;}�OH�Njά�����WK�-��a��+��
��;���A��&o�X��ֽ:8��ȫG�1�����o�9�
Еm<I0�^0��!�h���C�Â� #�
xb������n�W��B��[C��^w��_��jP�@Gb��˕Zͽ�6F�=���Z����w�}Pa]�E�g��5GG��p�AQ�H����PYV$�'�	C���,At�/��e�G2�>���Y�
(�\��>���?���}s����/L�[ه3��|Z	L(����R��]��Gip�%MX5M9�m����D�bWr �$zI��>��]l��;%�t��]=IW��{(hx��Jtde�c���9�E6�h�V�y#������/��J��d/#$�����`%c��-s�dP�E4�掶U։L��>6χ
B9{1t��ؙ�]7k+g���WV��J8Z����]��:��=��\��0�
UB��Sb���X�u0���ϼ��a��J����A70����$�1���}^2l�#�x͊Eٺ�P1��^=;C�����%�>ۑfHI����C�-M{�6�8��&̔\թ���S^�x����rsC�aif�ζ�i���B󭭰�3okq��~�g���v�2������?���-M��/��~%��������.-�tU�+v��a�>#Ä璅����qGx��}�� n�œ
������g�K9z(	��=ch_Xx(�ĺ��Xڈ¼����9�u��M�zl�G@}٪�囥�f�$�
���Td,���On�Ug�f9
�W��Xz)6®&�Y��I	O�/�
��;�L�����~��&LXP��� �-r[�S�6�c��)Se<��H�(����k�ے�&��\����_ǑZ�~�Y��+�~��0	[�JY�y��Q�h�ok�!�j�G�����t�,���ۨ;��}�_<�������5
���u�Z�Vn3�<x�rOT���� �W�����Xy��Ƈ\z�02
#�L�xn�טx�!���;��v�e�X�J���q�l�Y�TMh�-�r��
f����ak��{_�K��.�^1���&�D����=���pOl��/Lu���U�� �;��g�e��p�FeR��Ӽ��@��DQ��.��.Mi��,��(��&���'\D>��F�7H~WFs�=��6&xyS��sn�9�DMi%,���&��u��_���'d��?ԫߩ�B})\(ִ����������N{v�V�1����oǾ�]�)�����0)�V9����M�e��E>g���5�ulX���l���$��t$�P��į�2��~�g
��yG��:��~߳�:���:��>�j�U@nF`���l0�k����uy�ee!�?�Ir��-8����d�
�v-�L	�%7�V�v�z�	V%��,Ob(e����߃?�����4L�:Zy�H��"�XU�.5_(��*^��ʋ��Y�ǃ4��������Ytw��ƮR��gMT�����ї2�i�J�w��r��o��gX2a9���g
v�S����ϫ��U��������'t
�NY�ӭ5�q4
~�M~-�]�*��4���f���	��[)�><|	!+/����ϽZx�{@>J$�鬕1���C
�1���`9{�r]pB����ߠ�� �y��m��	�"�:.�Csz�y�6�b̂��|l������'�.ߢH���>������G��O��b��xL�����o2�l�-Sl]���8�ĵjD%�G�?�1M$/��(L�8ז?��yX':����;�d�95^������zz��i��;xً�9��iѐ��TŹW[?fV�	'Q��3T�g\���4���֞~����g9�򜊉�<(͂�!#�F�nk�^��]���ȿ���wts���/��LM�:����{����`����K�tI�m���v~ك��᭕�'�>o9��~�����z�t}�D�����[D��l�?B��_6��ot����I<W)_��/���+ة��"�nK��rc�o��$�̕��@O~h�i�[D�|�v�W���4�'K�E^o�@��
0]�B���H�c���̸juk�7i]�6�ߝ?�P�w��Ļ��O�U��W�DѵC�f�][�1��n��T��'hE�!2A�S��K��01��'P�ՇƖ��	_�g��+���q��Zz�O]{�
���f�E�i�2�t����'W��ڵ�b��PZ��V��Ts�����t��� m��P58��2�������\!��-����K�ȓ$7��:7���b�!�I ���-��h�>c������}F�6�X�8&��׹���`[�Hx��T���Ι��5�N�0*v��
�X���.�W�vz�G�h���M������h���ǣN�҈b�ƶ��ȵ!��쫑"�N�:ά��H��p����k���@�9�b�0����;O��fe�1f�(h��dQZE��7�S$3�Ft�w=+��VXդ%�s�>��<"�H�tyMI&�� �(g���[2�c�AaXN}���W�C��{��
	ҹ�Q*
pq-�q˾�)��i2Cj�Яkإ�G���R�
�;-��cu�^K��hSQקT`7]��A�o`�&fEg5R�~��a\�1�ZE^:��O�e�6w'���z�6��-'	�Ɠ<�m���ý�7Iz�K���o��؛��j?����ۦ�����(��ͬGx�����(��wZЕw�=��+�f��D��A�U�#��Q���#��J��ȗ�y�vzw�b��|��6؃��e��=�DU�վ������4lp��
� �]��@�ߪ���y��VԼܲrק�L�1�.��@G�^�5z̀��F�~�U�1uf ���9�+�5�`Ioo�p��9ޙ`rg�P
�G
j��=@����P�~�2��.�-��>�N��o^m����c���&�UN�4s��,z$���}���%F�+��S �G.�P?,����j�/���V��\	�����4&,�E:�j��w��Kڌ�nq_�B�8�`&���>c�P������i-�\ 3F>?�f�KQ�|�� ���y��dD"y��A
7�}@�auד�D^��o �Q�
����4򘷈�D}��q]��Ƽ�E ���H��H> Rf��L��4�ѪO�B>E��qY�#�}��\G���N��~�~!3�e$9gp�E6������U47�z��Թ��6�ߜ�I��)QH���>�G�a�1�αy v��K��J{9|�;�{)���˓�A�
�e�~�p��G�!v$w�*�����w,��cI�a8�J\�e�T*FJzc��R�
���xu*ϗ�/X����	IÉD|�2 &��������Cm�	]:*�~������Zش!w�S\;�w�ATT��p~�]v`aW�*F�$�SJ�P;r�y��l-�.���?A6fCn�hG0-�oеB�ڼ��%f�'�X>7B5�pgר�S��~x�b1��Q��`�3�2���=�!��:�[��çWׁ��I�粩����Q�p2kf9��/�?���!����v�_%t�r�D�
�~~ �z�e�S��MWZ/���m؋�ܮs���x��b /"b�Z���!��g�~��H�c���w��g0h��a�o�Dqqa��@�~A ��9��;�`�f�s��2���1erdrL�`�x
�JC\`t
3�oN��cl ��T����G���>3Ie��AUݮ����'����@OS�N����5�Z6qJ�h��i���1���87�u�6�l��1I���i �
į��QH2�� ut��?�:����sw�o��l5}��g���hU]���9�g�Oc�A������6	���;�W����2��cܯx'�.sc�����EH4%��o�tTEy�/Ya���#=U��W�Rhv��2��L��r�T�48������+����az�;�.�2� �)��FR;VDz�d ��l� lH
��H�~!�TE&�̄�;вo`���o��s
�J/g�s��QI��.��)S�_�.�G��]Sу�狺����.��4 ��%��T��� ��\k#qe(��+����܁�r��`��r�����	�V��,͛������"�%<���wss}|������=�K�����N�O��Mrb��9�	?��R?*��|hȗ�$[���}#��Y��%��s�p����i��ll �יhU��2SlT�WF͂�D�>i����[�1��5�dX1��0���uէ�!��y�h�K=�a�5���,�����G��X!��=��)b��W�rd�#�%`ꚮ���{Y���?��~�4<z,�6=�?'�]R�����-ʔɸ
��ɂ���KB�B� �6�_6,*���4$���0��=O��{�	�ˉP�\6�7����&�&���vf�`�L;����E	��_w<kƱ�tg�E�!�H���L�kU��,>�M��eV�sЯ�[�	������W���+�5�e�}���*<�@h<��B�B�T�ӭ�3�ھ[��; �k���%��(�b�n�:��������Ӎr]s����n\��ݎԉ'�?2�7��2�*韇o�f�Цl�w3�i�*4�0`#��x�\�R.oP�0J��?ߑv�l%�^���$��x��jt���$������!��"�0|Y57?v�=>:f�b$�%G8�wʟ�t��@~3�q��5/�Ar�B�}NV�J�� 7����m?�FbK�Ɏ�MP�#������A�$ݖ�JSz7`��8L�#��RYPC������a�~�|��Xk@�f:(F���W^	�xݤ��be'&.�#K�->����

e��bc@���nRԸW����2Zr�
�=��#�1Vn�h^׉�k��ID
=ܻ&ka�=�c :���÷pX.`�(�ѭ�Q�P>��Z(ԅ���� ~���d�� ���;\ǻ��[=�����a���P����O����]��ǋm���� �?�3�7w�[[�nc��m��������_M��iW�׿��߆��[5����ш��n���c_�é����=�=��"�?�38�O����I�u����OC��������ոw�ߒ}���ʒ��G����ȶ"�g0�o�����\��i�����s��� ��4~��Y�/��M��P�sJ�1v��_���P,��?�Z��_���C-&���4La����W��g0��&��|�?�����>}��}��t�G�����O`�ڠ})�����7=F�����y���������?��_����������X�������z�����'�2����*���������ɲ��6��2�ij�_����?�������uVk��'�_;����++�t}��?t��o�^�]w��4.?P�XNw�ڮ����ۡ��O���_����a������w)Ѥ.�b������������/����:����������%����Y�ʋ���9��w.t���s/F�w��*�������پ���+�����+�����+�����+�����+�����+�����+�C��AG�:  