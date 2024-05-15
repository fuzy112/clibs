#!/bin/bash
# Copyright © 2024  Zhengyi Fu <i@fuzy.me>

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

username=$(git config user.name)
year=$(date +%Y)
declare -a changed_files
git diff --cached --name-only >.file_list
val=0

while IPS='' read file; do

    # skip if the file is deleted
    [ ! -f file ] && continue

    sed -i "/Copyright.*${username}/{
/${year}/!{
    s/\\([0-9]\\+\\)\\(-[0-9]\\+\\)\\?/\\1-${year}/
        T
        H
    }
}
\${p; x; /./Q 1 ; Q  }" "$file"
    if [ $? != '0' ]; then
        val=1
    fi
done < .file_list

rm .file_list

exit $val
