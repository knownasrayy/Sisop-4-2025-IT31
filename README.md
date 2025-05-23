# Sisop-4-2025-IT31

- Erlangga Valdhio Putra Sulistio_5027241030
- Rayhan Agnan Kusuma_5027241102
- S. Farhan Baig_5027241097


# Soal 1_

## Deskripsi Soal
Shorekeeper menemukan teks hexadecimal anomali di Tethys' Deep. Tugas:
1. Ekstraksi file teks
2. Konversi hex ke gambar
3. Penamaan file dengan timestamp
4. Pencatatan log aktivitas

---

### **1a: Ekstraksi File Anomali**
**Tujuan**:  
Mengambil file ZIP dari sumber, mengekstrak ke direktori `anomali`, dan menghapus file ZIP.

**Langkah**:
```bash
# Salin file ZIP ke direktori kerja
cp '/path/anomali.zip' ~/blackshores/

# Unzip dan hapus file
unzip anomali.zip -d anomali
rm anomali.zip
