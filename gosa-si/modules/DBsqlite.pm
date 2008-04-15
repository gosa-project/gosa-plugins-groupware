package GOSA::DBsqlite;


use strict;
use warnings;
use DBI;
use Data::Dumper;
use GOSA::GosaSupportDaemon;
use Time::HiRes qw(usleep);


my $col_names = {};

sub new {
    my $class = shift;
    my $db_name = shift;

    my $lock = $db_name.".si.lock";
	# delete existing lock - instance should be running only once
	if(stat($lock)) {
		&main::daemon_log("DEBUG: Removed existing lock $lock.", 7);
		unlink($lock);
	}
    my $self = {dbh=>undef,db_name=>undef,db_lock=>undef,db_lock_handle=>undef};
    my $dbh = DBI->connect("dbi:SQLite:dbname=$db_name");
    $self->{dbh} = $dbh;
    $self->{db_name} = $db_name;
    $self->{db_lock} = $lock;
    bless($self,$class);

    return($self);
}

sub create_table {
    my $self = shift;
    my $table_name = shift;
    my $col_names_ref = shift;
    my @col_names;
    foreach my $col_name (@$col_names_ref) {
        my @t = split(" ", $col_name);
        $col_name = $t[0];
        push(@col_names, $col_name);
    }

    $col_names->{ $table_name } = $col_names_ref;
    my $col_names_string = join("', '", @col_names);
    my $sql_statement = "CREATE TABLE $table_name ( '$col_names_string' )"; 
    $self->{dbh}->do("DROP TABLE IF EXISTS $table_name");
    $self->{dbh}->do($sql_statement);
    return 0;
}


sub add_dbentry {
    my $self = shift;
    my $arg = shift;

    # if dbh not specified, return errorflag 1
    my $table = $arg->{table};
    if( not defined $table ) { 
        return 1 ; 
    }

    # specify primary key in table
    if (not exists $arg->{primkey}) {
        return (2, "a hash key 'primkey' with at least an empty list as value is necessary for add_dbentry");
    }
    my $primkeys = $arg->{'primkey'};
    my $prim_statement="";
    if( 0 != @$primkeys ) { 
        my @prim_list;
        foreach my $primkey (@$primkeys) {
            if($primkey eq 'id') {
                # if primkey is id, fetch max id from table and give new job id=  max(id)+1
                my $sql_statement = "SELECT MAX(CAST(id AS INTEGER)) FROM $table";
                my $max_id = @{ @{ $self->{dbh}->selectall_arrayref($sql_statement) }[0] }[0];
                my $id;
                if( defined $max_id) {
                    $id = $max_id + 1; 
                } else {
                    $id = 1;
                }
                $arg->{id} = $id;
            }
            if( not exists $arg->{$primkey} ) {
                return (3, "primkey '$primkey' has no value for add_dbentry");
            }
           push(@prim_list, "$primkey='".$arg->{$primkey}."'");
        }
        $prim_statement = "WHERE ".join(" AND ", @prim_list);
    }
 
    # if timestamp is not provided, add timestamp   
    if( not exists $arg->{timestamp} ) {
        $arg->{timestamp} = &get_time;
    }

    # check wether primkey is unique in table, otherwise return errorflag
    my $sql_statement = "SELECT * FROM $table $prim_statement";
    my $res = @{ $self->{dbh}->selectall_arrayref($sql_statement) };

    if ($res == 0) {
        # primekey is unique

        # fetch column names of table
        my $col_names = &get_table_columns($self, $table);

        # assign values to column name variables
        my @col_list;
		my @val_list;
        foreach my $col_name (@{$col_names}) {
            # use function parameter for column values
            if (exists $arg->{$col_name}) {
                push(@col_list, $col_name);
				push(@val_list, $arg->{$col_name});
            }
        }    

        my $sql_statement = "INSERT INTO $table (".join(", ", @col_list).") VALUES ('".join("', '", @val_list)."')";
	&main::daemon_log("DEBUG: $sql_statement", 7);
        my $db_res = $self->{dbh}->do($sql_statement);
        if( $db_res != 1 ) {
            return (4, $sql_statement);
        } 

    } else  {
        # entry already exists, so update it 
        my @update_l;
        while( my ($pram, $val) = each %{$arg} ) {
            if( $pram eq 'table' ) { next; }
            if( $pram eq 'primkey' ) { next; }
            push(@update_l, "$pram='$val'");
        }
        my $update_str= join(", ", @update_l);
        $update_str= " SET $update_str";

        my $sql_statement= "UPDATE $table $update_str $prim_statement";
        my $db_res = &update_dbentry($self, $sql_statement );

    }

    return 0;
}

sub update_dbentry {
    my ($self, $sql)= @_;
    my $db_answer= &exec_statement($self, $sql); 
    return $db_answer;
}


sub del_dbentry {
    my ($self, $sql)= @_;;
    my $db_res= &exec_statement($self, $sql);
    return $db_res;
}


sub get_table_columns {
    my $self = shift;
    my $table = shift;
    my @column_names;
    
    if(exists $col_names->{$table}) {
        @column_names = @{$col_names->{$table}};
    } else {
        my @res = @{$self->{dbh}->selectall_arrayref("pragma table_info('$table')")};

        foreach my $column (@res) {
            push(@column_names, @$column[1]);
        }
    }
    return \@column_names;

}


sub select_dbentry {
    my ($self, $sql)= @_;
    my $error= 0;
    my $answer= {};
    my $db_answer= &exec_statement($self, $sql); 
    my @column_list;

    # fetch column list of db and create a hash with column_name->column_value of the select query
    $sql =~ /SELECT ([\S\s]*?) FROM ([\S]*?)( |$)/g;
    my $selected_cols = $1;
    my $table = $2;

    # all columns are used for creating answer
    if ($selected_cols eq '*') {
        @column_list = @{ &get_table_columns($self, $table) };    

    # specific columns are used for creating answer
    } else {
        # remove all blanks and split string to list of column names
        $selected_cols =~ s/ //g;          
        @column_list = split(/,/, $selected_cols);
    }

    # create answer
    my $hit_counter = 0;
    my $list_len = @column_list;
    foreach my $hit ( @{$db_answer} ){
        $hit_counter++;
        for ( my $i = 0; $i < $list_len; $i++) {
            $answer->{ $hit_counter }->{ $column_list[$i] } = @{ $hit }[$i];
        }
    }

    return $answer;  
}


sub show_table {
    my $self = shift;
    my $table_name = shift;

    my $sql_statement= "SELECT * FROM $table_name ORDER BY timestamp";
    my $res= &exec_statement($self, $sql_statement);

    my @answer;
    foreach my $hit (@{$res}) {
        push(@answer, "hit: ".join(', ', @{$hit}));
    }
    return join("\n", @answer);
}


sub exec_statement {
    my $self = shift;
    my $sql_statement = shift;

    my @db_answer = @{$self->{dbh}->selectall_arrayref($sql_statement)};

    return \@db_answer;
}


sub exec_statementlist {
    my $self = shift;
    my $sql_list = shift;
    my @db_answer;

    foreach my $sql (@$sql_list) {
        @db_answer = @{$self->{dbh}->selectall_arrayref($sql)};
    }

    return \@db_answer;
}

sub count_dbentries {
    my ($self, $table)= @_;
    my $error= 0;
    my $answer= -1;
    
    my $sql_statement= "SELECT * FROM $table";
    my $db_answer= &select_dbentry($self, $sql_statement); 

    my $count = keys(%{$db_answer});
    return $count;
}



sub move_table {
    my ($self, $from, $to) = @_;

    my $sql_statement_drop = "DROP TABLE IF EXISTS $to";
    my $sql_statement_alter = "ALTER TABLE $from RENAME TO $to";
	$self->{dbh}->do($sql_statement_drop);
	$self->{dbh}->do($sql_statement_alter);

    return;
} 


1;
