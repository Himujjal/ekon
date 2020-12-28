type root = {
  unquotedKey: string;
  singleQuotes: string;
  multilineStrings: string;
  intNumber: number;
  floatingPointNumber: number;
  leadingDecimalPointNumber: number;
  trailingDecimlPointNumber: number;
  positiveSignNumber: number;
  hexadecimalNumber: number;
  arrays: [string, number, { key: string }, string[]];
  objectMap: { world: string; arr: string[]; anotherNumber: number };
  hello: { array: string; number: number };
  key: number;
  jsonObject: { key: string };
};
