let crypto = require('crypto');
let key = `1234567890123456789012345678901234567890123456789012345678901234`; //64位
let iv = `12345678901234567890123456789012`; //32位
function encryptAES256(data) {
  let cipher = crypto.createCipheriv('aes-256-cbc', Buffer.from(key, 'hex'), Buffer.from(iv, 'hex'));
  let encrypted = cipher.update(data, 'utf8', 'hex');
  encrypted += cipher.final('hex');
  return encrypted;
}
function decryptAES256(encryptedData) {
  let decipher = crypto.createDecipheriv('aes-256-cbc', Buffer.from(key, 'hex'), Buffer.from(iv, 'hex'));
  let decrypted = decipher.update(encryptedData, 'hex', 'utf8');
  decrypted += decipher.final('utf8');
  return decrypted;
}
console.log("key:",key)
console.log('IV:', iv);
let plaintext = 'This is a secret message';

// 加密数据
let encryptedData = encryptAES256(plaintext);
console.log('Encrypted Text:', encryptedData);

// 解密数据
let decrypted = decryptAES256(encryptedData);
console.log('Decrypted Text:', decrypted);