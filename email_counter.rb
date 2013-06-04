require 'net/imap'
require 'dino' # https://github.com/austinbv/dino

# open connection to the arduino
board = Dino::Board.new(Dino::TxRx.new)

while true

  factor = 10 # factor to reduce the number returned by
  delay  = 15 # seconds between queries
  pin    = 3  # must be a PWM pin


  # open connection to the mail server
  imap = Net::IMAP.new('imap.gmail.com', :ssl => { :verify_mode => OpenSSL::SSL::VERIFY_NONE })
  imap.login( 'YourEmail', 'YourPasswd')

  # get the number of unread emails from INBOX
  imap.examine('INBOX')
  ret = imap.status("inbox", ["UNSEEN"])
  imap.logout()
  imap.disconnect() 

  int = ret['UNSEEN']
  val = (int*factor).to_i

  # > 57 could damage the meter!
  if val < 57 
    board.analog_write(pin, val)
    puts ret
  else
    board.analog_write(pin, 57)
    puts "#{val} - Out of Range"
  end
 
  sleep delay
end
