package IP;
use strict;
use warnings;
use Digest::MD5 qw(md5_hex);

sub new {
    my ($class, %args) = @_;
    return bless \%args, $class;
}

sub conectar {
    my ($self) = @_;
    return unless $self->{conn};

    $self->_login();
    return unless $self->{password};
}

sub login {
    my ($self, $password) = @_;
    return unless $password;

    my $encrypted_password = md5_hex($password);
    $self->{password} = $encrypted_password;
    
    return $self;
}

sub _login {
    my ($self) = @_;
    return unless $self->{conn} && $self->{password};
    
    # Simulación de autenticación
    return 1 if $self->exec();
}

sub _repetir {
    my ($self, $timeout) = @_;
    
    while (1) {
        $self->set_timeout($timeout);
        last unless $self->exec();
    }
}

sub set_timeout {
    my ($self, $timeout) = @_;
    die "Error: Tiempo de espera inválido" unless defined $timeout;
    
    $self->{timeout} = $timeout;
}

sub exec {
    my ($self) = @_;
    return $self->_login() ? 1 : 0;
}

1;


package DDoS;
use strict;
use warnings;

sub new {
    my ($class, %args) = @_;
    return bless \%args, $class;
}

sub _repetir {
    my ($self) = @_;
    
    while (1) {
        $self->set_timeout(60);
        last unless $self->exec();
    }
}

sub set_timeout {
    my ($self, $timeout) = @_;
    die "Error: Tiempo de espera inválido" unless defined $timeout;
    
    $self->{timeout} = $timeout;
}

sub exec {
    my ($self) = @_;
    return 1;  # Simulación de ejecución exitosa
}

1;
