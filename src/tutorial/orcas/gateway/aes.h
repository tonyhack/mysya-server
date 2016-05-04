#ifndef TUTORIAL_ORCAS_GATEWAY_AES_H
#define TUTORIAL_ORCAS_GATEWAY_AES_H

namespace tutorial {
namespace orcas {
namespace gateway {

class Aes {
 public:
  explicit Aes(unsigned char *key);
  ~Aes();

  unsigned char *Encode(unsigned char *input);
  unsigned char *Decode(unsigned char *input);

  void *Encode(void *input, int length = 0);
  void *Decode(void *input, int length);

 private:
  void KeyExpansion(unsigned char* key, unsigned char w[][4][4]);
  unsigned char FFmul(unsigned char a, unsigned char b);

  void SubBytes(unsigned char state[][4]);
  void ShiftRows(unsigned char state[][4]);
  void MixColumns(unsigned char state[][4]);
  void AddRoundKey(unsigned char state[][4], unsigned char k[][4]);

  void InvSubBytes(unsigned char state[][4]);
  void InvShiftRows(unsigned char state[][4]);
  void InvMixColumns(unsigned char state[][4]);

  unsigned char box_[256];
  unsigned char inv_box_[256];
  unsigned char w_[11][4][4];
};

}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_AES_H
