const fs = require('fs');
const path = require('path');

const dds = process.env['DDS_ROOT'];
if (!fs.existsSync(path.join(dds, 'lib'))) {
    const prefix = path.dirname(path.dirname(dds));
    if (process.argv[2] === 'includes') {
        console.log(path.join(prefix, 'include'));
    } else if (process.argv[2] == 'libpaths') {
        console.log('-L' + path.join(prefix, 'lib'));
    }
}
