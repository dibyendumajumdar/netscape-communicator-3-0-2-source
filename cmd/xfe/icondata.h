/* -*- Mode: C; tab-width: 8 -*-
   icondata.h --- declarations for generated file icondata.c
   Copyright � 1997 Netscape Communications Corporation, all rights reserved.
   Created: Jamie Zawinski <jwz@netscape.com>, 28-Feb-95.
 */
#ifndef _ICONDATA_H_
#define _ICONDATA_H_

extern unsigned int fe_n_icon_colors;
extern unsigned short fe_icon_colors [][3];

#define MAX_ANIM_FRAMES 50
#define MAX_ANIMS 10

extern unsigned int fe_anim_frames[MAX_ANIMS];

struct fe_icon_data
{
  unsigned int width;
  unsigned int height;
  unsigned char *mono_bits;
  unsigned char *mask_bits;
  unsigned char *color_bits;
};

extern struct fe_icon_data BiffN;
extern struct fe_icon_data BiffU;
extern struct fe_icon_data BiffY;
extern struct fe_icon_data GAudio;
extern struct fe_icon_data GBinary;
extern struct fe_icon_data GFind;
extern struct fe_icon_data GFolder;
extern struct fe_icon_data GImage;
extern struct fe_icon_data GMovie;
extern struct fe_icon_data GTelnet;
extern struct fe_icon_data GText;
extern struct fe_icon_data GUnknown;
extern struct fe_icon_data HAddr;
extern struct fe_icon_data HArt;
extern struct fe_icon_data HBkm;
extern struct fe_icon_data HBkmCh;
extern struct fe_icon_data HBkmUnknown;
extern struct fe_icon_data HFolder;
extern struct fe_icon_data HFolderO;
extern struct fe_icon_data HGroup;
extern struct fe_icon_data HHdr;
extern struct fe_icon_data HHdrDest;
extern struct fe_icon_data HHdrDestO;
extern struct fe_icon_data HHdrMenu;
extern struct fe_icon_data HHdrMenuDest;
extern struct fe_icon_data HHdrMenuDestO;
extern struct fe_icon_data HHdrMenuO;
extern struct fe_icon_data HHdrO;
extern struct fe_icon_data HHost;
extern struct fe_icon_data HList;
extern struct fe_icon_data HListO;
extern struct fe_icon_data HMarked;
extern struct fe_icon_data HMsg;
extern struct fe_icon_data HRead;
extern struct fe_icon_data HSub;
extern struct fe_icon_data HUMarked;
extern struct fe_icon_data HURead;
extern struct fe_icon_data HUSub;
extern struct fe_icon_data IBad;
extern struct fe_icon_data IReplace;
extern struct fe_icon_data IconUnknown;
extern struct fe_icon_data MDel;
extern struct fe_icon_data MDel_i;
extern struct fe_icon_data MDel_pt;
extern struct fe_icon_data MDel_pt_i;
extern struct fe_icon_data MGetM;
extern struct fe_icon_data MGetM_i;
extern struct fe_icon_data MGetM_pt;
extern struct fe_icon_data MMarkA;
extern struct fe_icon_data MMarkA_i;
extern struct fe_icon_data MMarkA_pt;
extern struct fe_icon_data MMarkA_pt_i;
extern struct fe_icon_data MMarkT;
extern struct fe_icon_data MMarkT_i;
extern struct fe_icon_data MMarkT_pt;
extern struct fe_icon_data MMarkT_pt_i;
extern struct fe_icon_data MMsgFwd;
extern struct fe_icon_data MMsgFwd_i;
extern struct fe_icon_data MMsgFwd_pt;
extern struct fe_icon_data MMsgFwd_pt_i;
extern struct fe_icon_data MMsgNew;
extern struct fe_icon_data MMsgNew_i;
extern struct fe_icon_data MMsgNew_pt;
extern struct fe_icon_data MMsgNew_pt_i;
extern struct fe_icon_data MMsgPst;
extern struct fe_icon_data MMsgPst_i;
extern struct fe_icon_data MMsgPst_pt;
extern struct fe_icon_data MMsgPst_pt_i;
extern struct fe_icon_data MMsgPstF;
extern struct fe_icon_data MMsgPstF_i;
extern struct fe_icon_data MMsgPstF_pt;
extern struct fe_icon_data MMsgPstF_pt_i;
extern struct fe_icon_data MMsgPstR;
extern struct fe_icon_data MMsgPstR_i;
extern struct fe_icon_data MMsgPstR_pt;
extern struct fe_icon_data MMsgPstR_pt_i;
extern struct fe_icon_data MMsgRep;
extern struct fe_icon_data MMsgRep_i;
extern struct fe_icon_data MMsgRep_pt;
extern struct fe_icon_data MMsgRep_pt_i;
extern struct fe_icon_data MMsgRepA;
extern struct fe_icon_data MMsgRepA_i;
extern struct fe_icon_data MMsgRepA_pt;
extern struct fe_icon_data MMsgRepA_pt_i;
extern struct fe_icon_data MNextU;
extern struct fe_icon_data MNextU_i;
extern struct fe_icon_data MNextU_pt;
extern struct fe_icon_data MNextU_pt_i;
extern struct fe_icon_data MPrevU;
extern struct fe_icon_data MPrevU_i;
extern struct fe_icon_data MPrevU_pt;
extern struct fe_icon_data MPrevU_pt_i;
extern struct fe_icon_data MPrint;
extern struct fe_icon_data MStop;
extern struct fe_icon_data Shsecure;
extern struct fe_icon_data Sinsecure;
extern struct fe_icon_data Smix;
extern struct fe_icon_data Sreplace;
extern struct fe_icon_data Ssecure;
extern struct fe_icon_data TBack;
extern struct fe_icon_data TBack_i;
extern struct fe_icon_data TBack_pt;
extern struct fe_icon_data TBack_pt_i;
extern struct fe_icon_data TFind;
extern struct fe_icon_data TFind_pt;
extern struct fe_icon_data THome;
extern struct fe_icon_data THome_i;
extern struct fe_icon_data THome_pt;
extern struct fe_icon_data THome_pt_i;
extern struct fe_icon_data TLoadImages;
extern struct fe_icon_data TLoadImages_i;
extern struct fe_icon_data TLoadImages_pt;
extern struct fe_icon_data TLoadImages_pt_i;
extern struct fe_icon_data TNext;
extern struct fe_icon_data TNext_i;
extern struct fe_icon_data TNext_pt;
extern struct fe_icon_data TNext_pt_i;
extern struct fe_icon_data TOpenUrl;
extern struct fe_icon_data TOpenUrl_pt;
extern struct fe_icon_data TPrint;
extern struct fe_icon_data TPrint_pt;
extern struct fe_icon_data TReload;
extern struct fe_icon_data TReload_pt;
extern struct fe_icon_data TStop;
extern struct fe_icon_data TStop_i;
extern struct fe_icon_data TStop_pt;
extern struct fe_icon_data TStop_pt_i;
extern struct fe_icon_data TLogo;
extern struct fe_icon_data TLogo_pt;

/* Desktop icons */
extern struct fe_icon_data app;
extern struct fe_icon_data gold_app;
extern struct fe_icon_data DABook;
extern struct fe_icon_data DBookmark;
extern struct fe_icon_data DNews;
extern struct fe_icon_data DNoNewMail;
extern struct fe_icon_data DNewMail;
extern struct fe_icon_data DCompose;
extern struct fe_icon_data DEditor;

extern struct fe_icon_data anim_meteors_large[];
extern struct fe_icon_data anim_meteors_small[];
extern struct fe_icon_data anim_mozilla_large[];
extern struct fe_icon_data anim_mozilla_small[];
extern struct fe_icon_data anim_compass_large[];
extern struct fe_icon_data anim_compass_small[];
extern struct fe_icon_data* anim_custom_large;
extern struct fe_icon_data* anim_custom_small;
#ifdef __sgi
extern struct fe_icon_data anim_sgi_large[];
extern struct fe_icon_data anim_sgi_small[];
#endif /* __sgi */

extern struct fe_icon_data CSendM;
extern struct fe_icon_data CSendM_i;
extern struct fe_icon_data CSendM_pt;
extern struct fe_icon_data CSendM_pt_i;
extern struct fe_icon_data CAttachM;
extern struct fe_icon_data CAttachM_i;
extern struct fe_icon_data CAttachM_pt;
extern struct fe_icon_data CAttachM_pt_i;
extern struct fe_icon_data CAddress;
extern struct fe_icon_data CAddress_i;
extern struct fe_icon_data CAddress_pt;
extern struct fe_icon_data CAddress_pt_i;
extern struct fe_icon_data CQuote;
extern struct fe_icon_data CQuote_i;
extern struct fe_icon_data CQuote_pt;
extern struct fe_icon_data CQuote_pt_i;
extern struct fe_icon_data CSendLater;
extern struct fe_icon_data CSendLater_i;
extern struct fe_icon_data CSendLater_pt;
extern struct fe_icon_data CSendLater_pt_i;

extern struct fe_icon_data ed_browse;		
extern struct fe_icon_data ed_copy;		
extern struct fe_icon_data ed_cut;		
extern struct fe_icon_data ed_find;		
extern struct fe_icon_data ed_edit;		
extern struct fe_icon_data ed_new;		
extern struct fe_icon_data ed_new_i;		
extern struct fe_icon_data ed_new_pt;		
extern struct fe_icon_data ed_new_pt_i;		
extern struct fe_icon_data ed_open;		
extern struct fe_icon_data ed_paste;		
extern struct fe_icon_data ed_print;		
extern struct fe_icon_data ed_save;		

extern struct fe_icon_data ed_bold;
extern struct fe_icon_data ed_bold_i;
extern struct fe_icon_data ed_bullet;
extern struct fe_icon_data ed_bullet_i;
extern struct fe_icon_data ed_center;
extern struct fe_icon_data ed_center_i;
extern struct fe_icon_data ed_clear;
extern struct fe_icon_data ed_clear_i;
extern struct fe_icon_data ed_color;
extern struct fe_icon_data ed_color_i;
extern struct fe_icon_data ed_fixed;
extern struct fe_icon_data ed_fixed_i;
extern struct fe_icon_data ed_grow;
extern struct fe_icon_data ed_grow_i;
extern struct fe_icon_data ed_hrule;
extern struct fe_icon_data ed_hrule_i;
extern struct fe_icon_data ed_image;
extern struct fe_icon_data ed_image_i;
extern struct fe_icon_data ed_indent;
extern struct fe_icon_data ed_indent_i;
extern struct fe_icon_data ed_italic;
extern struct fe_icon_data ed_italic_i;
extern struct fe_icon_data ed_left;
extern struct fe_icon_data ed_left_i;
extern struct fe_icon_data ed_link;
extern struct fe_icon_data ed_link_i;
extern struct fe_icon_data ed_number;
extern struct fe_icon_data ed_number_i;
extern struct fe_icon_data ed_outdent;
extern struct fe_icon_data ed_outdent_i;
extern struct fe_icon_data ed_table;
extern struct fe_icon_data ed_table_i;
extern struct fe_icon_data ed_props;
extern struct fe_icon_data ed_props_i;
extern struct fe_icon_data ed_right;
extern struct fe_icon_data ed_right_i;
extern struct fe_icon_data ed_shrink;
extern struct fe_icon_data ed_shrink_i;
extern struct fe_icon_data ed_target;
extern struct fe_icon_data ed_target_i;

extern struct fe_icon_data ed_open_i;
extern struct fe_icon_data ed_save_i;
extern struct fe_icon_data ed_browse_i;
extern struct fe_icon_data ed_cut_i;
extern struct fe_icon_data ed_copy_i;
extern struct fe_icon_data ed_paste_i;
extern struct fe_icon_data ed_print_i;
extern struct fe_icon_data ed_find_i;
extern struct fe_icon_data ed_edit_i;
extern struct fe_icon_data ed_open_pt;
extern struct fe_icon_data ed_save_pt;
extern struct fe_icon_data ed_browse_pt;
extern struct fe_icon_data ed_cut_pt;
extern struct fe_icon_data ed_copy_pt;
extern struct fe_icon_data ed_paste_pt;
extern struct fe_icon_data ed_print_pt;
extern struct fe_icon_data ed_find_pt;
extern struct fe_icon_data ed_edit_pt;
extern struct fe_icon_data ed_open_pt_i;
extern struct fe_icon_data ed_save_pt_i;
extern struct fe_icon_data ed_browse_pt_i;
extern struct fe_icon_data ed_cut_pt_i;
extern struct fe_icon_data ed_copy_pt_i;
extern struct fe_icon_data ed_paste_pt_i;
extern struct fe_icon_data ed_print_pt_i;
extern struct fe_icon_data ed_find_pt_i;
extern struct fe_icon_data ed_edit_pt_i;
extern struct fe_icon_data ed_publish;
extern struct fe_icon_data ed_publish_i;
extern struct fe_icon_data ed_publish_pt;
extern struct fe_icon_data ed_publish_pt_i;
extern struct fe_icon_data ed_tag;
extern struct fe_icon_data ed_tage;
extern struct fe_icon_data ed_form;

extern struct fe_icon_data ImgB2B_r;	
extern struct fe_icon_data ImgB2B_d;	
extern struct fe_icon_data ImgB2D_r;	
extern struct fe_icon_data ImgB2D_d;	
extern struct fe_icon_data ImgC2B_r;	
extern struct fe_icon_data ImgC2B_d;	
extern struct fe_icon_data ImgC2C_r;	
extern struct fe_icon_data ImgC2C_d;	
extern struct fe_icon_data ImgWL_r;	
extern struct fe_icon_data ImgWL_d;	
extern struct fe_icon_data ImgWR_r;	
extern struct fe_icon_data ImgWR_d;	
extern struct fe_icon_data ImgT2T_r;	
extern struct fe_icon_data ImgT2T_d;	

extern struct fe_icon_data A_Signed;
extern struct fe_icon_data A_Encrypt;
extern struct fe_icon_data A_NoEncrypt;
extern struct fe_icon_data A_SignBad;
extern struct fe_icon_data A_EncrypBad;
extern struct fe_icon_data M_Attach;
extern struct fe_icon_data M_Signed;
extern struct fe_icon_data M_Encrypt;
extern struct fe_icon_data M_SignEncyp;
extern struct fe_icon_data M_SignBad;
extern struct fe_icon_data M_EncrypBad;
extern struct fe_icon_data M_SgnEncypBad;

#endif /*_ICONDATA_H_*/
