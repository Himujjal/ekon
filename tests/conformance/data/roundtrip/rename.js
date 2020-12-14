const fs = require("fs");

const allFiles = fs.readdirSync(".");
allFiles.forEach((file) => {
  const ext = file.slice(file.length - 5);
  const name = file.slice(0, file.length - 5);
  if (ext == ".json") {
    fs.renameSync("./" + file, "./" + name + ".ekon");
  }
});

// delete all JSON files
allFiles.forEach((file) => {
  const ext = file.slice(file.length - 5);
  if (ext == ".json") {
    fs.rmSync(file);
  }
});

console.log(fs.readdirSync("."));
