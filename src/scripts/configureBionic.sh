# run this in script to configure Ubuntu 18.04 with the tools needed to build xlang
#   sudo bash configBionic.sh 

# Note, in order to run in WSL, this file *MUST* have Unix-style LF line endings.
#       If you get errors like "'\r': command not found", this file has been mistakenly
#       updated to have Windows-style CRLF line endings.
#       .gitattributes file in this repo has been updated to ensure all .sh files 
#       have LF only line endings.

# update APT packages
sudo apt update 
sudo apt upgrade -y
sudo apt autoremove -y

# install clang, cmake, ninja and libc++ via APT
sudo apt install clang cmake ninja-build libc++-dev libc++abi-dev -y