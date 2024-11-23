git submodule update --init --recursive --remote || (echo "Error while fetching submodules" && exit 1)

cd scripts || exit 2
  ln -sf ../dependencies/Utils/scripts/check.py ./
  ln -sf ../dependencies/Utils/scripts/file_validator.py ./
  ln -sf ../dependencies/Utils/scripts/project_config.py ./
  ln -sf ../dependencies/Utils/scripts/project_utils.py ./
  ln -sf ../dependencies/Utils/scripts/project_validator.py ./
  ln -sf ../dependencies/Utils/scripts/string_utils.py ./
  ln -sf ../dependencies/Utils/scripts/git_hooks ./
  python check.py || (echo "Impossible to continue installing. Errors were occurred" && exit 3)
  ./install_boost.sh
cd ../
