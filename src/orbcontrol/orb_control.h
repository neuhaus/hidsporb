#ifndef ORB_CONTROL_H
#define ORB_CONTROL_H

/*
  A basic class for controlling the orb.  This should be
  treated as a fairly opaque beastie--in other words, use
  the publicly-available functions, but don't worry too much
  about how it's implemented, because I fully expect that to
  change in the future.  This version, published with the initial
  hidsporb beta driver, communicates with the drivec via the registry.
*/
class orb_control_implementation;

class orb_control {
 public:
  orb_control( void );
  ~orb_control( void );

 public:
  enum facing { Horizontal_facing, Vertical_facing, Custom_facing };

  enum error { No_error,
	       Could_not_open_base_key,
	       Could_not_open_subkey };

  //commands
  void set_axis( int logical_axis, int physical_axis );
  void set_chording( int use_chording );
  void set_facing( facing new_facing );
  void set_null_region( int new_null_region );
  void set_sensitivity( int logical_axis, int sensitivity );
  void set_polarity( int logical_axis, int polarity );
  void set_gain( int logical_axis, int gain );
  void set_precision_sensitivity( int sensitivity );
  void set_precision_gain( int gain );
  void set_precision_button( int button_type, int button_index );
  void upload_sensitivity_curve( int curve_id, unsigned short* buffer );
  void set_defaults( void );

  //queries
  int is_initialized( void );
  int physical_axis_from_logical_axis( int logical_axis );
  int using_chording( void );
  int null_region( void );
  facing current_facing( void );
  int sensitivity( int logical_axis );
  int polarity( int logical_axis );
  int gain( int logical_axis );
  int precision_sensitivity( void );
  int precision_gain( void );
  int precision_button_type( void );
  int precision_button_index( void );
  void download_sensitivity_curve( int curve_id, unsigned short* buffer );

  //validity queries
  int is_valid_axis( int axis );
  int is_valid_sensitivity( int sensitivity );
  int is_valid_polarity( int polarity );
  int is_valid_gain( int gain );
  int is_valid_button_type( int button_type );
  int is_valid_physical_button_index( int button_index );
  int is_valid_logical_button_index( int button_index );
  int is_valid_null_region( int new_null_region );
  int is_valid_curve_id( int new_curve_id );
  int is_valid_curve( unsigned short* buffer );

  error last_error( void );
  const char* last_error_string( void );

 private:
  orb_control_implementation* m_pimp;
};


#endif
