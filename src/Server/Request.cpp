/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maraasve <maraasve@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 15:14:13 by maraasve          #+#    #+#             */
/*   Updated: 2025/04/07 15:54:46 by maraasve         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./Request.hpp"

Request::Request(std::string& request) {
	if (request.empty()) {
		_error_code = "400";
		return ;
	}
}
